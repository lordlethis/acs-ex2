//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "node.h"
#include "protocol_m.h"

/** constants for self-messages */
#define DO_JOIN_MSG   "GET_AN_ID"
#define DO_LEAVE_MSG  "LEAVE_NETWORK"
#define CHECK_HEARTBEAT_MSG "CHECK_PULSE"

static AcsMessage* copyMessage(AcsMessage* msg);

/**
 * Helper object to remember the last heart beat's sequence number
 * from when the self-message was fired off.
 * (we always schedule a message to ourselves, telling us to check
 *  whether a new heart beat arrived in the mean time. So this
 *  object's number together with the value maintained in the object
 *  itself will tell us whether we did get a heart beat.)
 */
class HeartControl : public cObject
{
public:
	HeartControl(long _seq) : cObject(), seq(_seq) {}
	long seq;
};

// Register module class with OMNeT++
Define_Module(IdNode);

void IdNode::initialize()
{
	id = NULL;
	hasId = false;
	// make stuff observable in UI
	WATCH_PTR(id);
	WATCH(hasId);
	WATCH(beatInterval);
	WATCH(prevBeatSeq);
	WATCH(prevBeatTime);
	// read params from ned/ini file
	minKeepIdTime = par("minKeepIdTime");
	maxKeepIdTime = par("maxKeepIdTime");
	retryTime = par("retryTime");
	rejoinDelay = par("rejoinDelay");
	initialDelay = par("initialDelay");
	if (initialDelay < 0) // random delay as default
		initialDelay = getRNG(0)->intRand()%5000;
	cMessage* msg = new cMessage(DO_JOIN_MSG);
	scheduleAt(simTime()+initialDelay, msg);
}

/**
 * Returns a object we can stream log messages into. The message is prepended with
 * this nodes name & index
 */
cEnvir& IdNode::log()
{
	return (EV << "(" << getName() << "[" << getIndex() << "]" << ") ");
}

/**
 * Setter method indicating whether the node has an id, i.e. whether it is
 * associated with the network.
 * As a side effect, the node's color is changed in the UI to reflect the
 * association state (gray <-> disconnected, green <-> connected).
 */
void IdNode::setHasId(bool has)
{
	hasId = has;
	if (hasId)
	{
		setDisplayString("i=block/routing,green");
		firstBeat = true;
	}
	else
	{
		setDisplayString("i=block/routing,gray");
	}
}

void IdNode::scheduleHeartBeatCheck()
{
	cMessage* msg = new cMessage(CHECK_HEARTBEAT_MSG);
	msg->setControlInfo(new HeartControl(prevBeatSeq));
	scheduleAt(prevBeatTime+beatInterval, msg);
}

void IdNode::handleMessage(cMessage *msg)
{
	// check whether we got a self-message telling us to join the network
	if (msg->getName() != NULL && !strcmp(msg->getName(),DO_JOIN_MSG))
	{
		if (!hasId) // we ignore this self-message if we're already associated
		{
			// prepare an AcquireId message and send it out
			AcquireId *amsg = new AcquireId("ACQUIRE_ID");
			if (id != NULL)
			{
				amsg->setId(Identifier(id->id));
				amsg->setHasId(true);
				id = NULL;
			}
			acquireMessageId = (((simTime().raw()) << 32) & 0xFFFFFFFF00000000) + getRNG(0)->intRand();
			amsg->setMessageId(acquireMessageId);
			for (int i = 0; i < gateSize("gate"); ++i)
				send(copyMessage(amsg), "gate$o", i);
			delete amsg;
			// schedule next attempt should we not get an id...
			cMessage* dmsg = new cMessage(DO_JOIN_MSG);
			scheduleAt(simTime()+retryTime,dmsg);
		}
		delete msg;
	}
	// check if we got a self-message telling us to drop out of the network (because we can)
	else if (msg->getName() && !strcmp(msg->getName(), DO_LEAVE_MSG))
	{
		// drop out of network, schedule reassociation
		log() << "Leaving the network\n";
		dropout = NULL;
		setHasId(false);
		cMessage* jmsg = new cMessage(DO_JOIN_MSG);
		simtime_t delay = rejoinDelay;
		scheduleAt(simTime()+delay,jmsg);
		delete msg;
	}
	// check if we got a self-message telling us to check the heart beat reception
	else if (msg->getName() && !strcmp(msg->getName(), CHECK_HEARTBEAT_MSG))
	{
		HeartControl *ctrl = (HeartControl*)msg->getControlInfo();
		if (hasId && ctrl->seq == prevBeatSeq)
		{
			// we've lost contact :'(
			// drop from the network...
			if (dropout)
				cancelAndDelete(dropout);
			dropout = NULL;
			// schedule reassociation
			cMessage* jmsg = new cMessage(DO_JOIN_MSG);
			simtime_t delay = rejoinDelay;
			scheduleAt(simTime()+delay,jmsg);
			setHasId(false);
			log() << "Dropped out of the network...";
		}
		delete msg;
	}
	else
	{
		bool broadcast = true;
		AcsMessage *tmsg = check_and_cast<AcsMessage*>(msg);

		// abort if we've already touched this msg
		if (hasId) {
			for (size_t i = 0; i < tmsg->getPath().size(); ++i)
			{
				if (tmsg->getPath()[i] == *id)
				{
					delete msg;
					return;
				}
			}
		}

		switch (tmsg->getMsgType())
		{
		case PING:
			// ignore ping if we're not associated
			if (hasId)
			{
				if (((Ping*)tmsg)->getId().id == id->id)
				{
					// we were pinged. respond with pong :D
					Pong* pong = new Pong("PONG");
					pong->setId(id->id);
					cGate* gate = msg->getArrivalGate();
					send(pong,"gate$o",gate->getIndex());
					broadcast = false;
				}
			}
			break;
		case ID_ASSIGNMENT:
			// if we have an id, don't check if the message is for this node
			if (!hasId && ((IdAssignment*)tmsg)->getMessageId() == this->acquireMessageId)
			{
				if (id != NULL)
					delete id;
				// remember the id we just obtained
				id = new Identifier(((IdAssignment*)tmsg)->getId().id);
				setHasId(true);
				log() << "got id \"" << id->id << "\"\n";
				// fetch heart beat information
				prevBeatSeq = ((IdAssignment*)tmsg)->getLastHeartBeat();
				beatInterval = ((IdAssignment*)tmsg)->getBeatInterval();
				beatInterval = beatInterval+beatInterval/4;
				// BEGIN: new in task 3
				// schedule heartbeat check
				prevBeatTime = simTime();
				scheduleHeartBeatCheck();
				// END: new in task 3

				// schedule disassociation
				cMessage* lmsg = dropout = new cMessage(DO_LEAVE_MSG);
				simtime_t delay = getRNG(0)->intRand(maxKeepIdTime-minKeepIdTime)+minKeepIdTime;
				scheduleAt(simTime()+delay,lmsg);
				broadcast = false;
			}
			break;
		// BEGIN: introduced in task 3
		case HEARTBEAT:
			if (hasId)
			{
				HeartBeat *beat = (HeartBeat*)msg;
				if (beat->getSeq() == prevBeatSeq+1)
				{
					// everything is working as it should
					prevBeatSeq++;
					prevBeatTime = simTime();
					scheduleHeartBeatCheck();
				}
				else if (beat->getSeq() < prevBeatSeq+1)
				{
					// seems to be an old one - remove it from circulation
					broadcast = false;
				}
				else //if (beat->getSeq() > prevBeatSeq)
				{
					// we normally shouldn't hit this code, but one never knows...
					// especially while debugging :D
					log() << "We have missed a beat. Disassociating...\n";
					setHasId(false);
					broadcast = false;
				}
			}
			break;
		// END: introduced in task 3
		case ACQUIRE_ID:
		case PONG:
			// ignore & relay
			break;
		default:
			log() << "Got a message of unknown type \"" << tmsg->getMsgType() << "\". Ignoring...\n";
		}

		// broadcast code
		if (hasId && broadcast)
		{
			// send a copy of the message on all gates except for the input gate of the message
			int idx = tmsg->getArrivalGate()->getIndex();
			tmsg->getPath().push_back(*id);
			for (int i = 0; i < gateSize("gate"); ++i) {
				if (i == idx) continue;
				AcsMessage *m = copyMessage(tmsg);
				if (m)
				{
					send(m, "gate$o",i);
				}
			}
		}

		// clean up
		if (msg)
			delete msg;
	}
}

/**
 * Copy messages ... The stupid way, without using cMessage::dup() ... *sigh*
 * @deprecated
 */
static AcsMessage* copyMessage(AcsMessage *msg)
{
	AcsMessage* result = msg->dup();
	if (result) {
		result->getPath() = msg->getPath();
		if (result->getPath().size() != msg->getPath().size())
			fputs("ERROR: path assignment failed :x", stderr);
	}
	return result;
}

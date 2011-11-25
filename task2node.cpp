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

#include "task2node.h"
#include "protocol1_m.h"

#define DO_JOIN_MSG   "GET_AN_ID"
#define DO_LEAVE_MSG  "LEAVE_NETWORK"
#define CHECK_HEARTBEAT_MSG "CHECK_PULSE"


static Task1Message* copyMessage(Task1Message* msg);

class HeartControl : public cObject
{
public:
	HeartControl(long _seq) : cObject(), seq(_seq) {}
	long seq;
};

// Register module class with OMNeT++
Define_Module(Task2Node);

void Task2Node::initialize()
{
	id = NULL;
	hasId = false;
	WATCH_PTR(id);
	WATCH(hasId);
	WATCH(beatInterval);
	WATCH(prevBeatSeq);
	WATCH(prevBeatTime);
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

cEnvir& Task2Node::log()
{
	return (EV << "(" << getName() << "[" << getIndex() << "]" << ") ");
}


void Task2Node::setHasId(bool has)
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

void Task2Node::scheduleHeartBeatCheck()
{
	cMessage* msg = new cMessage(CHECK_HEARTBEAT_MSG);
	msg->setControlInfo(new HeartControl(prevBeatSeq));
	scheduleAt(prevBeatTime+beatInterval, msg);
}

void Task2Node::handleMessage(cMessage *msg)
{
	if (msg->getName() != NULL && !strcmp(msg->getName(),DO_JOIN_MSG))
	{
		if (!hasId)
		{
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
	else if (msg->getName() && !strcmp(msg->getName(), DO_LEAVE_MSG))
	{
		log() << "Leaving the network\n";
		dropout = NULL;
		setHasId(false);
		cMessage* jmsg = new cMessage(DO_JOIN_MSG);
		simtime_t delay = rejoinDelay;
		scheduleAt(simTime()+delay,jmsg);
		delete msg;
	}
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
		Task1Message *tmsg = check_and_cast<Task1Message*>(msg);

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
			if (hasId)
			{
				if (((Ping*)tmsg)->getId().id == id->id)
				{
					Pong* pong = new Pong("PONG");
					pong->setId(id->id);
					cGate* gate = msg->getArrivalGate();
					send(pong,"gate$o",gate->getIndex());
					broadcast = false;
				}
			}
			break;
		case ID_ASSIGNMENT:
			if (!hasId && ((IdAssignment*)tmsg)->getMessageId() == this->acquireMessageId)
			{
				if (id != NULL)
					delete id;
				id = new Identifier(((IdAssignment*)tmsg)->getId().id);
				setHasId(true);
				log() << "got id \"" << id->id << "\"\n";
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
			int idx = tmsg->getArrivalGate()->getIndex();
			tmsg->getPath().push_back(*id);
			for (int i = 0; i < gateSize("gate"); ++i) {
				if (i == idx) continue;
				Task1Message *m = copyMessage(tmsg);
				if (m)
				{
					send(m, "gate$o",i);
				}
			}
		}

		if (msg)
			delete msg;
	}
}

static Task1Message* copyMessage(Task1Message *msg)
{
	Task1Message* result;
	switch (msg->getMsgType())
	{
	case PING: {
		result = new Ping(*((Ping*)msg));
		break;
		}
	case PONG: {
		result = new Pong(*((Pong*)msg));
		break;
		}
	case ID_ASSIGNMENT: {
		result = new IdAssignment(*((IdAssignment*)msg));
		break;
		}
	case ACQUIRE_ID: {
		result = new AcquireId(*((AcquireId*)msg));
		break;
		}
	case HEARTBEAT: { // yeah, that case is new, too
		result = new HeartBeat(*((HeartBeat*)msg));
		break;
	}
	default: {
		result = NULL;
		break;
		}
	}
	if (result) {
		result->getPath() = msg->getPath();
		if (result->getPath().size() != msg->getPath().size())
			fputs("ERROR: path assignment failed :x", stderr);
	}
	return result;
}

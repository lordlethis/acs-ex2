#include "idserver.h"
#include <string.h>
#include "protocol_m.h"
#include <stdio.h>

// Register module class with OMNeT++
Define_Module(IdServer);

#define DELAY_MESSAGE "delaymsg"
#define INIT_TICTOC_MSG "init_tictoc"
#define RELESE_TICTOC "relese_tictoc"

class IdControl : public cObject
{
public:
	IdControl(t1id_t _id) : cObject(), id(_id) {}
	t1id_t id;
};

void IdServer::initialize()
{
	// read params from ned/ini file
	timeout = par("timeout");
	rangeStart = par("range_start");
	rangeEnd = par("range_end");
	pulseRate = par("pulse_rate");
	// read params form ned/ini file for the tictoc init
    ticStart = par("tictoc_start");
	ticLoop = par("tictoc_interval");
	ticTurn = par("tictoc_lasts");

    // take the first id of the range for ourselves
	id = new Identifier(rangeStart++);
	nextId = rangeStart;
	// allow inspection in UI
	WATCH(nextId);
	WATCH(id);
	WATCH(rangeStart);
	WATCH(rangeEnd);
	WATCH(pulseRate);
	WATCH(ticStart);
	EV << "Task2Server initialized\n";
	lastBeat = simTime()+pulseRate;
	scheduleAt(lastBeat, &fireBeat);

	// setting timer for the start of initialization for the TIC-TOC
	cMessage* msg = new cMessage(INIT_TICTOC_MSG);
	scheduleAt(simTime()+ticStart, msg);
}


cEnvir& IdServer::log()
{
	return (EV << "(" << getName()  << ") ");
}

void IdServer::handleSelfMessage(cMessage *msg)
{
	// check if we got a delay message
	// delay messages are badly named, but upon reception, we check if
	// we received a PONG for the ping of some id...
	if (msg->getName() != NULL && !strcmp(msg->getName(),DELAY_MESSAGE))
	{
		t1id_t _id = ((IdControl*)(msg->getControlInfo()))->id;
		log() << "Handling delay msg for id \"" << _id << "\"...\n";
		// pendingIds contains _id, i.e. we did not receive a PONG --> we can give this ID to a client :D
		if (pendingIds.find(_id) != pendingIds.end())
		{
			AcquireId* amsg = (AcquireId*)pendingIds[_id];
			pendingIds.erase(_id);
			cGate* gate = amsg->getArrivalGate();
			IdAssignment* reply = new IdAssignment("ID_ASSIGNMENT");
			reply->setId(Identifier(_id));
			reply->setMessageId(amsg->getMessageId());
			reply->setLastHeartBeat(heartBeat-1);
			reply->setBeatInterval(pulseRate);
			std::stringstream ss;
			gate->getName();
			ss << gate->getBaseName() << "$o";
			send(reply,ss.str().c_str(),gate->getIndex());
			delete amsg;
		}
		delete msg;
	}
	else if (msg->getName() != NULL && !strcmp(msg->getName(),DO_PULSE_MSG))
	{
		// fire another heart beat, pumping blood through the veins
		HeartBeat* beat;
		for (int i = 0; i < gateSize("gate"); ++i)
		{
			beat = new HeartBeat;
			beat->setSeq(heartBeat);
			send(beat,"gate$o",i);
		}
		++heartBeat;
		lastBeat += pulseRate;
		scheduleAt(lastBeat, &fireBeat);
	}
	//initializing tictoc
	else if (msg->getName() != NULL && !strcmp(msg->getName(),INIT_TICTOC_MSG))
	{
		// choosing two random nodes for tictoc
		int roundsize = nextId - 1;

		// finding tic node that is not already in action
		int tic;
		do
		{
			tic = rand() % roundsize + rangeStart;
		}
		while(ticNodes.find(tic)!=ticNodes.end());
		ticNodes[tic]=tic;

		// finding toc node that is not already in action
		int toc;
		do
		{
		toc = rand() % roundsize + rangeStart;
		}
		while(ticNodes.find(toc)!=ticNodes.end());
		ticNodes[toc]=toc;

		log() << "sending tictoc initialization to tic node: \n";
		int ngates = gateSize("gate");
		for (int i = 0; i < ngates; ++i)
		{
			TicInit *tInit = new TicInit("TICTOC_INIT",TICTOC_INIT);
			tInit->getPath().push_back(*getId());
			tInit->setId(Identifier(tic));
			tInit->setTocId(Identifier(toc));
			send(tInit, "gate$o", i);
		}
		// setting the timer for the next initialization
		cMessage* msg = new cMessage(INIT_TICTOC_MSG);
		scheduleAt(simTime()+ticLoop, msg);

		TicTurn* turn = new TicTurn(RELESE_TICTOC);
		turn->setTic(tic);
		turn->setToc(toc);
		scheduleAt(simTime()+ticTurn, turn);
	}
	// Release tic and toc nodes in the internal log because they are finished
	else if (msg->getName() != NULL && !strcmp(msg->getName(),RELESE_TICTOC))
	{
		TicTurn *turn = check_and_cast<TicTurn*>(msg);
		MapType::iterator iter1 = ticNodes.find(turn->getTic());
		if(iter1!=ticNodes.end())ticNodes.erase(iter1);

		MapType::iterator iter2 = ticNodes.find(turn->getToc());
		if(iter2!=ticNodes.end())ticNodes.erase(iter2);
	}

}

CommonNode::HandlingState IdServer::handleUncommonMessage(cMessage *msg)
{
	log() << "Handling real msg...\n";
	AcsMessage *tmsg = check_and_cast<AcsMessage*>(msg);
	CommonNode::HandlingState state = HandlingStates::UNHANDLED;
	switch (tmsg->getMsgType())
	{
	case PONG:
		{
			Pong *pong = (Pong*)tmsg;
			t1id_t _id = pong->getId().id;
			pendingIds.erase(_id);
			state = HandlingStates::HANDLED | HandlingStates::FORWARD;
		}
	break;
	case ACQUIRE_ID:
		{
			// a client wants a new ID
			state = HandlingStates::HANDLED;
			bool foundId = false;
			t1id_t attempts = 0;
			t1id_t _id;
			AcquireId* amsg = check_and_cast<AcquireId*>(msg);
			// did we get this request before? if so, ignore it
			for (PendingIdMap::iterator iter = pendingIds.begin(); iter != pendingIds.end(); ++iter)
			{
				AcquireId* a = dynamic_cast<AcquireId*>(iter->second);
				if (a && a->getMessageId() == amsg->getMessageId())
				{
					return HandlingStates::HANDLED;
				}
			}
			// maybe the client wished for a specific id?
			_id = amsg->getId().id;
			foundId = amsg->getHasId();
			// find an id that may be free (if we do not already have one)
			while (!foundId && (rangeEnd-rangeStart) >= attempts)
			{
				_id = nextId;
				if (nextId == rangeEnd)
					nextId = rangeStart;
				else
					nextId++;
				if (!(pendingIds.find(_id) != pendingIds.end()))
					foundId = true;
				else
					++attempts;
			}
			if (!foundId)
			{
				// tough luck ... but clients can try again :D
				break;
			}

			// send ping through all gates to find out if the id is actually free
			log() << "probing for id \"" << _id << "\"\n";
			int ngates = gateSize("gate");
			for (int i = 0; i < ngates; ++i)
			{
				Ping *ping = new Ping("PING",PING);
				ping->getPath().push_back(*getId());
				ping->setId(Identifier(_id));
				send(ping, "gate$o", i);
			}

			// store id/message pair in map for later retrieval (to respond)
			pendingIds[_id] = msg;
			// send msg to self to respond to the client at some point
			cMessage* dmsg = new cMessage(DELAY_MESSAGE);
			dmsg->setControlInfo(new IdControl(_id));
			scheduleAt(simTime()+timeout, dmsg);
			state |= HandlingStates::NODELETE;
			log() << "Returning handling state: " << state;
			break;
		}
	}
	return state;
}

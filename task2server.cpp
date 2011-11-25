#include "task2server.h"
#include <string.h>
#include "protocol1_m.h"
#include <stdio.h>

// Register module class with OMNeT++
Define_Module(Task2Server);

#define DELAY_MESSAGE "delaymsg"

class IdControl : public cObject
{
public:
	IdControl(t1id_t _id) : cObject(), id(_id) {}
	t1id_t id;
};

void Task2Server::initialize()
{
	timeout = par("timeout");
	rangeStart = par("range_start");
	rangeEnd = par("range_end");
	pulseRate = par("pulse_rate");
	id = new Identifier(rangeStart++);
	nextId = rangeStart;
	WATCH(nextId);
	WATCH(id);
	WATCH(rangeStart);
	WATCH(rangeEnd);
	WATCH(pulseRate);
	EV << "Task2Server initialized\n";
	lastBeat = simTime()+pulseRate;
	scheduleAt(lastBeat, &fireBeat);
}


cEnvir& Task2Server::log()
{
	return (EV << "(" << getName()  << ") ");
}


void Task2Server::handleMessage(cMessage *msg)
{
//	cGate *gate = msg->getArrivalGate();
//	EV << "Got message on gate \"" << gate->getDisplayString() << "\".\n";
	if (msg->getName() != NULL && !strcmp(msg->getName(),DELAY_MESSAGE))
	{
		t1id_t _id = ((IdControl*)(msg->getControlInfo()))->id;
		log() << "Handling delay msg for id \"" << _id << "\"...\n";
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
	else
	{
		log() << "Handling real msg...\n";
		Task1Message *tmsg = check_and_cast<Task1Message*>(msg);
		switch (tmsg->getMsgType())
		{
		case PONG:
			{
				Pong *pong = (Pong*)tmsg;
				t1id_t _id = pong->getId().id;
				pendingIds.erase(_id);
				delete pong;
			}
		break;
		case ACQUIRE_ID:
			{
				bool foundId = false;
				t1id_t attempts = 0;
				t1id_t _id;
				AcquireId* amsg = check_and_cast<AcquireId*>(msg);
				// did we get this request before?
				for (boost::unordered_map<t1id_t, cMessage*>::iterator iter = pendingIds.begin(); iter != pendingIds.end(); ++iter)
				{
					AcquireId* a = dynamic_cast<AcquireId*>(iter->second);
					if (a && a->getMessageId() == amsg->getMessageId())
					{
						delete amsg;
						return;
					}
				}
				// maybe the client wished for a specific id?
				_id = amsg->getId().id;
				foundId = amsg->getHasId();
				// find an id that may be free o.o
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
					delete msg;
					break;
				}

				// send ping through all gates
				int ngates = gateSize("gate");
				for (int i = 0; i < ngates; ++i)
				{
					Ping *ping = new Ping("PING",PING);
					ping->setId(Identifier(_id));
					send(ping, "gate$o", i);
				}
				// send msg to self to respond to the client at some point
				log() << "probing for id \"" << _id << "\"\n";
				pendingIds[_id] = msg;
				cMessage* dmsg = new cMessage(DELAY_MESSAGE);
				dmsg->setControlInfo(new IdControl(_id));
				scheduleAt(simTime()+timeout, dmsg);
				break;
			}
		}
	}
}

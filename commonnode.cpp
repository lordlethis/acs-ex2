/*
 * commonnode.cpp
 *
 *  Created on: Nov 26, 2011
 *      Author: kei
 */

#include "commonnode.h"
#include "types.h"
#include "protocol.h"
#include "protocol_m.h"


CommonNode::CommonNode() {
}

CommonNode::~CommonNode() {
}

void CommonNode::initialize()
{
	helloId = 0;
	helloInterval = par("helloInterval");
	routingTableTimeout = par("routingTableTimeout");
	ticTocDuration = par("ticTocDuration");
}

void CommonNode::startHelloProtocol()
{
	if (!sendHelloMsg.isScheduled())
	{
		scheduleAt(simTime()+helloInterval,&sendHelloMsg);
	}
}

void CommonNode::handleMessage(cMessage *msg)
{
	if (msg->isSelfMessage())
	{
		if (!handleSelfMessage(msg))
		{
			// if true is returned, msg should also be cleaned up already
			EV << "WARNING: unhandled self-message! \"" << msg->getName() << "\"";
			delete msg;
		}
	}
	else
	{
		// short check for a cycling packet (removing such abominations)
		AcsMessage* amsg = dynamic_cast<AcsMessage*>(msg);
		if (amsg)
		{
			if (hasId())
			{
				for (size_t i = 0; i < amsg->getPath().size(); ++i)
				{
					if (amsg->getPath()[i] == *getId())
					{
						delete msg;
						return;
					}
				}
			}
		}
		HandlingState h = handleUncommonMessage(msg);
		EV << "uncommon message handling returned: " << h << "\n";
		if (!(h & HandlingStates::HANDLED))
		{
			h |= handleCommonMessage(msg);
		}
		if (amsg && (h & HandlingStates::FORWARD))
		{
			// broadcast code
			if (hasId())
			{
				// send a copy of the message on all gates except for the input gate of the message
				int idx = amsg->getArrivalGate()->getIndex();
				amsg->getPath().push_back(*getId());
				for (int i = 0; i < gateSize("gate"); ++i) {
					if (i == idx) continue;
					AcsMessage *m = amsg->dup();
					if (m)
					{
						send(m, "gate$o",i);
					}
				}
			}
		}
		if (!(h & HandlingStates::NODELETE))
			delete msg;
	}
}

bool CommonNode::handleSelfMessage(cMessage* msg)
{
	if (msg == &sendHelloMsg)
	{
		if (hasId())
		{
			for (int i = 0; i < gateSize("gate"); ++i)
			{
				HelloMessage *hmsg = new HelloMessage();
				hmsg->getPath().push_back(*getId());
				hmsg->setMessageId(helloId);
				send(hmsg,"gate$o",i);
			}
			++helloId;
		}
		std::vector<Identifier> killList;
		for (RoutingTable::iterator iter = routingTable.begin(); iter != routingTable.end(); ++iter)
		{
			if (simTime()-iter->second.lastUpdate > routingTableTimeout)
				killList.push_back(Identifier(iter->first));
		}
		for (std::vector<Identifier>::iterator iter = killList.begin(); iter != killList.end(); ++iter)
		{
			routingTable.erase(*iter);
		}
		scheduleAt(simTime()+helloInterval,&sendHelloMsg);
		return true;
	}
	return false;
}

CommonNode::HandlingState CommonNode::handleCommonMessage(cMessage* msg)
{
	AcsMessage* amsg = dynamic_cast<AcsMessage*>(msg);
	CommonNode::HandlingState state = HandlingStates::UNHANDLED;
	switch (amsg->getMsgType())
	{
	case PING:
	{
		// ignore ping if we're not associated
		if (hasId())
		{
			if (((Ping*)amsg)->getId().id == getId()->id)
			{
				// we were pinged. respond with pong :D
				Pong* pong = new Pong("PONG");
				pong->setId(getId()->id);
				pong->getPath().push_back(*getId());
				cGate* gate = msg->getArrivalGate();
				send(pong,"gate$o",gate->getIndex());
				state = HandlingStates::HANDLED;
			}
			else
			{
				state = HandlingStates::FORWARD | HandlingStates::HANDLED;
			}
		}
		break;
	} // end PING
	case ACQUIRE_ID:
	case PONG:
	{
		state = HandlingStates::FORWARD;
		break;
	} // end PONG / ACQUIRE_ID
	case HELLO:
	{
		Identifier& source = *amsg->getPath().begin();
		if (routingTable.find(source) != routingTable.end())
		{
			// is this message old? if so, do nothing, especially not forward!
			if (routingTable[source].lastId >= ((HelloMessage*)amsg)->getMessageId() && (routingTable[source].lastId - ((HelloMessage*)amsg)->getMessageId()) < 1e10L)
			{
				state = HandlingStates::HANDLED;
				break;
			} else {
				routingTable[source].lastId = ((HelloMessage*)amsg)->getMessageId();
			}
		}
		state = HandlingStates::FORWARD | HandlingStates::HANDLED;
		int hops = amsg->getPath().size();
		for (PacketPath::iterator iter = amsg->getPath().begin(); iter != amsg->getPath().end(); ++iter)
		{
			if (routingTable.find(*iter) == routingTable.end())
			{
				// no entry yet - insert one
				routingTable[*iter] = RoutingEntry(*iter,amsg->getArrivalGate()->getIndex(),hops,simTime(),0L);
				if (hops == 1)
					routingTable[*iter].lastId = ((HelloMessage*)amsg)->getMessageId();
			}
			else
			{
				if (routingTable[*iter].nhops > hops)
				{
					routingTable[*iter] = RoutingEntry(*iter,amsg->getArrivalGate()->getIndex(),hops,simTime(),routingTable[*iter].lastId);
				}
				else if (routingTable[*iter].nhops == hops && routingTable[*iter].gateNum == amsg->getArrivalGate()->getIndex())
				{
					routingTable[*iter].lastUpdate = simTime();
				}
			}
			hops--;
		}
		break;
	} // end HELLO
	case ROUTABLE:
	{
		EV << "Handling routable...";
		if (!hasId())
			break;
		RoutableMessage* rmsg = (RoutableMessage*)msg;
		if (rmsg->getTarget() != *getId())
		{
			EV << "Trying to forward it...\n";
			state = HandlingStates::HANDLED | HandlingStates::NODELETE;
			if (!routeMessage(rmsg))
			{
				EV << "Couldn't route message for target " << rmsg->getTarget().id;
			}
		}
		else
		{
			EV << "It seems I'm it's destination.\n";
			state = handleRoutableMessage(rmsg);
		}
		break;
	} // end ROUTABLE
	} // end switch msgtype
	return state;
}

CommonNode::HandlingState CommonNode::handleRoutableMessage(RoutableMessage* msg)
{
	CommonNode::HandlingState state = HandlingStates::UNHANDLED;
	if (msg->getPayload())
	{
		switch (msg->getPayload()->getPayloadType()) {
		case 0: // TICTOC
			{
				EV << "It's a TICTOC!\n";
				state = HandlingStates::HANDLED;
				TicToc* tt = (TicToc*)msg->getPayload();
				if (tt->endTime > simTime())
				{
					state |= HandlingStates::NODELETE;
					Identifier src = msg->getSource();
					msg->getPath().clear();
					msg->getPath().push_back(*getId());
					msg->setTarget(src);
					msg->setSource(*getId());
					EV << "Trying to send it back!\n";
					routeMessage(msg);
				} else {
					EV << "It shall rest in peace now.\n";
					EV << "EndTime: " << tt->endTime << " / currentTime: " << simTime() << "\n";
				}
			}
			break;
		case 1: // Initiate TICTOC
			{
				EV << "It's a call for TICTOC!\n";
				RoutableMessage *rmsg = new RoutableMessage("TICTOC");
				rmsg->getPath().push_back(*getId());
				rmsg->setPayload(new TicToc(simTime()+ticTocDuration));
				EV << "endTime: " << simTime()+ticTocDuration << "\n";
				EV << "duration: " << ticTocDuration << "\n";
				rmsg->setTarget(((InitiateTicToc*)msg->getPayload())->target);
				rmsg->setSource(*getId());
				routeMessage(rmsg);
				state = HandlingStates::HANDLED;
			}
			break;
		default:
			EV << "WTF is this?? " << msg->getPayload()->getPayloadType() << "\n";
		}
	}
	return state;
}

bool CommonNode::routeMessage(RoutableMessage *msg)
{
	if (!hasId() || msg->getTarget() == *getId()) // return false if we're disconnected or the actual target.
	{
		EV << "Cannot route packet - we have no id or we're the target\n";
		return false;
	}
	RoutingTable::iterator iter = routingTable.find(msg->getTarget());
	if (iter == routingTable.end()) // we don't know the target
	{
		EV << "Cannot route packet: target " << msg->getTarget().id << "not known.\n";
		return false;
	}
	RoutingEntry &e = iter->second;
	send(msg,"gate$o",e.gateNum);
	return true;
}

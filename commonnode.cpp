/*
 * commonnode.cpp
 *
 *  Created on: Nov 26, 2011
 *      Author: kei
 */

#include "commonnode.h"
#include "protocol_m.h"
#include "idcollector.h"

CommonNode::CommonNode() {
}

CommonNode::~CommonNode() {
}

void CommonNode::initialize()
{
	int x = par("x");
	int y = par("y");
	id = Identifier(x,y);
	IdCollector::instance()->addId(id);
}

void CommonNode::handleMessage(cMessage *msg)
{
	if (msg->isSelfMessage())
	{
		handleSelfMessage(msg);
	}
	else
	{
		AcsMessage* amsg = dynamic_cast<AcsMessage*>(msg);
		HandlingState h = handleUncommonMessage(msg);
		if (!(h & HandlingStates::HANDLED))
		{
			h |= handleCommonMessage(msg);
		}
		if (amsg && (h & HandlingStates::BROADCAST))
		{
			// broadcast code
			if (hasId())
			{
				// send a copy of the message on all gates except for the input gate of the message
				int idx = amsg->getArrivalGate()->getIndex();
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

CommonNode::HandlingState CommonNode::handleCommonMessage(cMessage* msg)
{
	AcsMessage* amsg = dynamic_cast<AcsMessage*>(msg);
	CommonNode::HandlingState state = HandlingStates::UNHANDLED;
	switch (amsg->getMsgType())
	{
	case HELLO:
	{
		HelloMessage* hmsg = (HelloMessage*)amsg;
		Identifier source = hmsg->getSource();
		neighbours[hmsg->getArrivalGate()->getIndex()] = source;
		break;
	}
	case ROUTABLE:
	{
		RoutableMessage* rmsg = (RoutableMessage*)amsg;
		if (rmsg->getTarget() == getId())
		{
			handleRoutableMessage(rmsg);
		}
		else
		{
			forwardMessage(rmsg);
		}
		state = HandlingStates::HANDLED | HandlingStates::NODELETE;
	}
	default:
		break;
	} // end switch msgtype
	return state;
}

void CommonNode::handleRoutableMessage(RoutableMessage *msg)
{
	if (!msg->getPayload())
		return;
	switch (msg->getPayload()->getPayloadType())
	{
	case 0: // TICTOC
	{
		TicTocPayload* tt = (TicTocPayload*)msg->getPayload();
		if (tt->endTime >= simTime())
		{
			log() << "Tictoc should end... Byebye... :)" << "\n";
			delete tt;
			delete msg;
			return;
		}
		msg->setTarget(msg->getSource());
		msg->setSource(getId());
		forwardMessage(msg);
	}
	default:
		log() << "unknown payload type " << msg->getPayload()->getPayloadType() << "\n";
	}
}

void CommonNode::forwardMessage(RoutableMessage* msg)
{
	int gateNum = -1;
	int minDist = msg->getTarget().distance(getId());
	for (NeighbourList::iterator iter = neighbours.begin(); iter!=neighbours.end(); ++iter)
	{
		int distance = iter->second.distance(msg->getTarget());
		if (distance < minDist)
		{
			minDist = distance;
			gateNum = iter->first;
		}
	}
	if (gateNum != -1)
		send(msg,"gate$o",gateNum);
	else
		log() << "Dropping routable message - there's no neighbour that is closer to the target than we are already. Gready routing fails." << "\n";
}

cEnvir& CommonNode::log()
{
	return ( EV << "[" << getName() << " - " << getId().info() << "] " );
}

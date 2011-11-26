/*
 * commonnode.cpp
 *
 *  Created on: Nov 26, 2011
 *      Author: kei
 */

#include "commonnode.h"
#include "types.h"
#include "protocol_m.h"

CommonNode::CommonNode() {
}

CommonNode::~CommonNode() {
}

void CommonNode::handleMessage(cMessage *msg)
{
	if (msg->isSelfMessage())
	{
		handleSelfMessage(msg);
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
	} // end PONG
	} // end switch msgtype
	return state;
}

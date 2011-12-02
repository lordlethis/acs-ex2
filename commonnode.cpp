/*
 * commonnode.cpp
 *
 *  Created on: Nov 26, 2011
 *      Author: kei
 */

#include "commonnode.h"
#include "protocol_m.h"
#include <algorithm>
#include <sstream>
#include <vector>
#include <limits.h>
#include <cstdlib>

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
		AcsMessage* amsg = dynamic_cast<AcsMessage*>(msg);
		HandlingState h = handleUncommonMessage(msg);
		if (!(h & HandlingStates::HANDLED))
		{
			h |= handleCommonMessage(msg);
		}
		if (amsg && (h & HandlingStates::BROADCAST))
		{
			// send a copy of the message on all gates except for the input gate of the message
			int idx = amsg->getArrivalGate()->getIndex();
			for (int i = 0; i < gateSize("gate"); ++i) {
				if (i == idx) continue;
				send(amsg->dup(), "gate$o",i);
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
	case LM_BROADCAST:
	{
		LandmarkBroadcast* lmb = (LandmarkBroadcast*)msg;
		int hc = lmb->getHopCount();
		int lmId = lmb->getLandmarkId();
		lmb->setHopCount(hc+1);
		state = HandlingStates::BROADCAST | HandlingStates::HANDLED;
		if (getId().hasLandmarkId(lmId) && getId()[lmId] <= hc)
		{
			// remove this from circulation because it's either cycling or irrelevant
			state = HandlingStates::HANDLED;
			break;
		}
		getId()[lmId] = hc;
		HelloMessage hmsg;
		hmsg.setSource(getId());
		broadcastMessage(&hmsg);

		break;
	}
	case HELLO:
	{
		GateId gid = msg->getArrivalGate()->getIndex();
		neighbours[gid] = ((HelloMessage*)msg)->getSource();
		state = HandlingStates::HANDLED;
		break;
	}
	case ROUTABLE:
	{
		RoutableMessage* rmsg = (RoutableMessage*)msg;
		state = HandlingStates::HANDLED;
		if (std::find(rmsg->getPath().begin(),rmsg->getPath().end(),getId()) != rmsg->getPath().end())
		{
			EV << "detected a routing loop... losing message now.\n";
			break;
		}

		rmsg->getPath().push_back(getId());
		if (rmsg->getTarget()==getId())
		{
			handleRoutableMessage(rmsg);
		} else {
			forwardMessage(rmsg);
		}
		break;
	}
	} // end switch msgtype
	return state;
}

void CommonNode::broadcastMessage(cMessage *msg)
{
	for (int i = 0; i < gateSize("gate"); ++i)
	{
		send(msg->dup(), "gate$o", i);
	}
}

void CommonNode::handleRoutableMessage(RoutableMessage *msg)
{
}

void CommonNode::forwardMessage(RoutableMessage* msg)
{
	GateId closestGate = NULL;
	int minDistance = INT_MAX;
	Identifier &id = getId();
	for (NeighbourList::iterator iter = neighbours.begin(); iter != neighbours.end(); ++iter)
	{
		int d = 0;
		const Identifier &nid = iter->second;
		const Identifier::IdentMap imap = nid.getIds();
		if (imap.size() != id.getIds().size()) // make sure we have the same amount of
			continue;
		for (Identifier::IdentMap::const_iterator idit = imap.begin(); idit != imap.end(); ++idit)
		{
			int ld1 = idit->second;
			if (!id.hasLandmarkId(idit->first))
			{
				d = INT_MAX;
				idit = imap.end(); // end loop
			}
			else
			{
				int ld2 = id[idit->first];
				int n = abs(ld1 - ld2);
				d += n*n;
			}
		}
		if (d < minDistance)
		{
			minDistance = d;
			closestGate = iter->first;
		}
	}
	if (closestGate)
		send(msg,closestGate);
}

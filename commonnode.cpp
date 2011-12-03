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
		state = handleLandmarkBroadcast((LandmarkBroadcast*)msg);
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
			log() << "detected a routing loop... losing message now.\n";
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

CommonNode::HandlingState CommonNode::handleLandmarkBroadcast(LandmarkBroadcast* lmb)
{
	// check if we've seen this message before
	if (std::find(lmMsgIds.begin(), lmMsgIds.end(), lmb->getMessageId()) != lmMsgIds.end())
	{
		log() << "removing old lm broadcast from circulation" << "\n";
		return HandlingStates::HANDLED;
	}
	lmMsgIds.push_back(lmb->getId());
	if (lmMsgIds.size() > 20)
		lmMsgIds.erase(lmMsgIds.begin());
	int hc = lmb->getHopCount()+1;
	int lmId = lmb->getLandmarkId();
	log() << "Got a lm beacon: LMID = " << lmId << ", HC = " << hc << "\n";
	lmb->setHopCount(hc);
	if (getId().hasLandmarkId(lmId) && getId()[lmId] < hc)
	{
		// remove this from circulation because it's irrelevant
		return HandlingStates::HANDLED;
	}
	getId()[lmId] = hc;
	HelloMessage hmsg;
	hmsg.setSource(getId());
	broadcastMessage(&hmsg);
	return HandlingStates::BROADCAST | HandlingStates::HANDLED;
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
	log() << "Got a routable, but I don't handle them yet..." << "\n";
}

void CommonNode::forwardMessage(RoutableMessage* msg)
{
	GateId closestGate = 0;
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

cEnvir& CommonNode::log() const
{
        return (EV << "(" << getName()  << ") ");
}

std::string CommonNode::info() const
{
	std::stringstream ss;
	ss << getId().info();
	return ss.str();
}

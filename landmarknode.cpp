/*
 * LandmarkNode.cpp
 *
 *  Created on: Dec 2, 2011
 *      Author: kei
 */

#include "landmarknode.h"
#include "protocol_m.h"

LandmarkNode::LandmarkNode() {
}

LandmarkNode::~LandmarkNode() {
}

// Register module class with OMNeT++
Define_Module(LandmarkNode);

void LandmarkNode::initialize()
{
	lmId = par("landmarkId");
	getId()[lmId] = 0;
	broadcastLandmark = new cMessage("LANDMARK_BROADCAST");
	scheduleAt(simTime()+100,broadcastLandmark);
	WATCH(lmId);
}

void LandmarkNode::handleSelfMessage(cMessage *msg)
{
	if (msg == broadcastLandmark)
	{
		LandmarkBroadcast lmb;
		lmb.setLandmarkId(lmId);
		lmb.setHopCount(0);
		long id = (simTime().raw() << 32)+getRNG(0)->intRand();
		lmb.setMessageId(id);
		broadcastMessage(&lmb);
		scheduleAt(simTime()+2,broadcastLandmark);
	}
}

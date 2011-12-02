/*
 * LandmarkNode.h
 *
 *  Created on: Dec 2, 2011
 *      Author: kei
 */

#ifndef LANDMARKNODE_H_
#define LANDMARKNODE_H_

#include "commonnode.h"
#include <omnetpp.h>

class LandmarkNode: public CommonNode {
public:
	LandmarkNode();
	virtual ~LandmarkNode();
	virtual void initialize();
protected:
	virtual void handleSelfMessage(cMessage *msg);
	virtual CommonNode::HandlingState handleUncommonMessage(cMessage *msg) { return HandlingStates::UNHANDLED; }
private:
	cMessage* broadcastLandmark;
	int lmId;
};

#endif /* LANDMARKNODE_H_ */

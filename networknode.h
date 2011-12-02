/*
 * NetworkNode.h
 *
 *  Created on: Dec 2, 2011
 *      Author: kei
 */

#ifndef NETWORKNODE_H_
#define NETWORKNODE_H_

#include "commonnode.h"

class NetworkNode: public CommonNode {
public:
	NetworkNode();
	virtual ~NetworkNode();
protected:
	virtual void handleSelfMessage(cMessage *msg);
	virtual CommonNode::HandlingState handleUncommonMessage(cMessage* msg) { return HandlingStates::UNHANDLED; }
};

#endif /* NETWORKNODE_H_ */

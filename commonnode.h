/*
 * commonnode.h
 *
 *  Created on: Nov 26, 2011
 *      Author: kei
 */

#ifndef COMMONNODE_H_
#define COMMONNODE_H_

#include <omnetpp.h>

class Identifier;


struct HandlingStates {
	static const unsigned int UNHANDLED = 0;
	static const unsigned int HANDLED = 1;
	static const unsigned int FORWARD = 2;
	static const unsigned int NODELETE = 4;
};

class CommonNode : public cSimpleModule {
public:
	CommonNode();
	virtual ~CommonNode();

	virtual void handleMessage(cMessage *msg);
	typedef unsigned int HandlingState;
protected:
	/**
	 * Handle messages, not including message disposal.
	 * This method is called from within #handleMessage(cMessage*)
	 */
	virtual HandlingState handleUncommonMessage(cMessage *msg) = 0;
	/**
	 * Handle self messages, including disposal if necessary
	 * This method is called from within #handleMessage(cMessage*)
	 */
	virtual void handleSelfMessage(cMessage *msg) = 0;
	virtual Identifier* getId() = 0;
	virtual bool hasId() = 0;
private:
	HandlingState handleCommonMessage(cMessage *msg);
};

#endif /* COMMONNODE_H_ */

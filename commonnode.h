/*
 * commonnode.h
 *
 *  Created on: Nov 26, 2011
 *      Author: kei
 */

#ifndef COMMONNODE_H_
#define COMMONNODE_H_

#include <omnetpp.h>
#include "routingtable.h"

class Identifier;


struct HandlingStates {
	static const unsigned int UNHANDLED = 0;
	static const unsigned int HANDLED = 1;
	static const unsigned int FORWARD = 2;
	static const unsigned int NODELETE = 4;
};

class RoutableMessage;

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
	virtual bool handleSelfMessage(cMessage *msg);
	virtual void initialize();
	virtual Identifier* getId() = 0;
	virtual bool hasId() = 0;
	void startHelloProtocol();
	bool routeMessage(RoutableMessage* msg);
	RoutingTable routingTable;
private:
	HandlingState handleCommonMessage(cMessage *msg);
	cMessage sendHelloMsg;
	long helloId;
	simtime_t helloInterval;
	simtime_t routingTableTimeout;
};

#endif /* COMMONNODE_H_ */

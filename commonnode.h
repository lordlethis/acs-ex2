/*
 * commonnode.h
 *
 *  Created on: Nov 26, 2011
 *      Author: kei
 */

#ifndef COMMONNODE_H_
#define COMMONNODE_H_

#include <omnetpp.h>
#include <boost/unordered_map.hpp>
#include "protocol.h"

class RoutableMessage;
typedef int GateId;


struct HandlingStates {
	static const unsigned int UNHANDLED = 0;
	static const unsigned int HANDLED = 1;
	static const unsigned int BROADCAST = 2;
	static const unsigned int NODELETE = 4;
};

class CommonNode : public cSimpleModule {
public:
	CommonNode();
	virtual ~CommonNode();

	virtual void handleMessage(cMessage *msg);
	typedef unsigned int HandlingState;
	typedef boost::unordered_map<GateId,Identifier> NeighbourList;
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
	virtual Identifier& getId() { return _id; }
	virtual const Identifier& getId() const { return _id; }
	virtual bool hasId() { return true; }
	virtual void forwardMessage(RoutableMessage* msg);
private:
	Identifier _id;
	HandlingState handleCommonMessage(cMessage *msg);
	NeighbourList neighbours;
};

#endif /* COMMONNODE_H_ */

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
#include <string.h>
#include <string>
#include <vector>

class RoutableMessage;
class LandmarkBroadcast;

struct HandlingStates {
	static const unsigned int UNHANDLED = 0;
	static const unsigned int HANDLED = 1;
	static const unsigned int BROADCAST = 2;
	static const unsigned int NODELETE = 4;
};

/*struct GateId {
	GateId(const char* _name, const int num) : name(_name) , gateNum(num) {}
	GateId(const GateId& other) : name(other.name) , gateNum(other.gateNum) {}
	~GateId() { delete name; }
	const char* name;
	bool operator==(const GateId& other) const { return strcmp(name,other.name)==0; }
	const int gateNum;
};*/
typedef int GateId;

class CommonNode : public cSimpleModule {
public:
	CommonNode();
	virtual ~CommonNode();

	virtual void handleMessage(cMessage *msg);
	typedef unsigned int HandlingState;
	typedef boost::unordered_map<GateId,Identifier> NeighbourList;
	virtual std::string info() const;
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
	Identifier& getId() { return _id; }
	const Identifier& getId() const { return _id; }
	void forwardMessage(RoutableMessage* msg);
	void handleRoutableMessage(RoutableMessage* msg);
	void broadcastMessage(cMessage* msg);
	cEnvir& log() const;
private:
	HandlingState handleLandmarkBroadcast(LandmarkBroadcast* lmsg);
	HandlingState handleCommonMessage(cMessage *msg);

	Identifier _id;
	NeighbourList neighbours;
	std::vector<long> lmMsgIds;
};

#endif /* COMMONNODE_H_ */

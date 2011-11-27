/*
 * task1server.h
 *
 *  Created on: Oct 11, 2011
 *      Author: kei
 */

#ifndef TASK2SERVER_H_
#define TASK2SERVER_H_

#include <omnetpp.h>
#include "commonnode.h"
#include <unordered_map>
#include "types.h"

#define DO_PULSE_MSG "BLOODFLOW"

class Identifier;

typedef std::unordered_map<t1id_t, cMessage*> PendingIdMap;

class IdServer : public CommonNode
{
public:
	IdServer() : heartBeat(0),fireBeat(DO_PULSE_MSG),ticTocInitiation("START_TICTOC") {}
protected:
    // The following redefined virtual function holds the algorithm.
    virtual void initialize();
    virtual bool handleSelfMessage(cMessage *msg);
    virtual HandlingState handleUncommonMessage(cMessage *msg);
    virtual bool hasId() { return true; }
    virtual Identifier* getId() { return id; }
private:
    cEnvir& log();
    Identifier* id;
    /** the amount of time we wait for a PONG */
    int timeout;
    /**
     * Heart beat counter
     */
    long heartBeat;
    /**
     * The interval in which heart beats are fired
     */
    int pulseRate;
    /**
     * The time when our previous heart beat fired
     */
    simtime_t lastBeat;
    /**
     * A self-message to remind our heart to beat...
     */
    cMessage fireBeat;
    /**
     * Start of the ID range
     */
    t1id_t rangeStart;
    /**
     * End of the ID range
     */
    t1id_t rangeEnd;
    /**
     * Candidate for the next id. We just count up each time and hope nobody is using this... ^.^
     */
    t1id_t nextId;
    /**
     * Map candidate ID to message - contains all IDs that still wait for a PONG or timeout
     */
    PendingIdMap pendingIds;
    cMessage ticTocInitiation;
    long ticTocStart;
    long ticTocInterval;
};


#endif /* TASK2SERVER_H_ */

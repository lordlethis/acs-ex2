/*
 * task1server.h
 *
 *  Created on: Oct 11, 2011
 *      Author: kei
 */

#ifndef TASK2SERVER_H_
#define TASK2SERVER_H_

#include <omnetpp.h>
#include <boost/unordered_map.hpp>
#include "types.h"

#define DO_PULSE_MSG "BLOODFLOW"

class Identifier;

class Task2Server : public cSimpleModule
{
public:
	Task2Server() : heartBeat(0),fireBeat(DO_PULSE_MSG) {}
protected:
    // The following redefined virtual function holds the algorithm.
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
private:
    cEnvir& log();
    Identifier* id;
    int timeout;
    /** @since task 3 */
    long heartBeat;
    /** @since task 3 */
    int pulseRate;
    /** @since task 3 */
    simtime_t lastBeat;
    t1id_t rangeStart, rangeEnd;
    t1id_t nextId;
    boost::unordered_map<t1id_t, cMessage*> pendingIds;
    /** @since task 3 */
    cMessage fireBeat;
};


#endif /* TASK2SERVER_H_ */

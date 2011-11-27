/*
 * routingtable.h
 *
 *  Created on: Nov 27, 2011
 *      Author: kei
 */

#ifndef ROUTINGTABLE_H_
#define ROUTINGTABLE_H_

#include <boost/unordered_map.hpp>
#include "protocol.h"
#include "protocol.h"

struct RoutingEntry {
	RoutingEntry() {}
	RoutingEntry(Identifier _target, int _gateNum, int _nhops, simtime_t time, long id) : target(_target),gateNum(_gateNum),nhops(_nhops),lastUpdate(time),lastId(id) {}
	Identifier target;
	int gateNum;
	int nhops;
	simtime_t lastUpdate;
	long lastId;
};

typedef boost::unordered_map<t1id_t,RoutingEntry> RoutingTable;


#endif /* ROUTINGTABLE_H_ */

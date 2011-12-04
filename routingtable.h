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

/**
 * Routing table will look like the following...
+-------------+------+-------+------------+--------+
| target node | gate | nhops | lastUpdate | lastId |
+-------------+------+-------+------------+--------+
 *
 * The lastUpdate field is used to remove stale entries (i.e. ones we didn't get a refresh after some time)
 * The lastId field is used to remove the broadcasted messages early on (i.e. before they did a full cycle) - so it's just some optimization
 */

struct RoutingEntry {
	RoutingEntry() {}
	RoutingEntry(Identifier _target, int _gateNum, int _nhops, simtime_t time, long id) : target(_target),gateNum(_gateNum),nhops(_nhops),lastUpdate(time),lastId(id) {}
	RoutingEntry(const RoutingEntry& other) : target(other.target), gateNum(other.gateNum), nhops(other.nhops), lastUpdate(other.lastUpdate), lastId(other.lastId) {}
	Identifier target;
	int gateNum;
	int nhops;
	simtime_t lastUpdate;
	long lastId;
};

typedef boost::unordered_map<t1id_t,RoutingEntry> RoutingTable;


#endif /* ROUTINGTABLE_H_ */

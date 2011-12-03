#ifndef _TASK1_PROTOCOL_
#define _TASK1_PROTOCOL_

#include "types.h"
#include <vector>
#include <omnetpp.h>
#include <sstream>

/**
 * Stupid wrapper class around identifiers. Initially implemented to allow using __int128,
 * and left in due to laziness... ^.^
 */
struct Identifier : public cObject
{
	Identifier() : id(0) {}
	Identifier(const Identifier& other) : id(other.id) {}
	Identifier(t1id_t _id) : id(_id) {}
	virtual std::string info() const { std::stringstream out; out << id; return out.str(); }
	virtual std::string detailedInfo() const { return info(); }
	t1id_t id;
	bool operator ==(const Identifier& other) const { return other.id == id; }
};
/*Structure that stores node and arriving gate to it*/
struct RouteStep : public cObject
{
	RouteStep() : id(0), gateNum(0) {}
	RouteStep(const RouteStep& other) : id(other.id), gateNum(other.gateNum) {}
	RouteStep(t1id_t _id, int _gateNum) : id(_id), gateNum(_gateNum) {}
	virtual std::string info() const { std::stringstream out; out << id.id << ", " << gateNum; return out.str(); }
	virtual std::string detailedInfo() const { return info(); }

	Identifier id;
	int gateNum;
};
/**
 * Stupid wrapper for simTime
 */
struct SiTime : public cObject
{
	SiTime() : time(0){}
	SiTime(const SiTime& st) : time(st.time){}
	SiTime(SimTime _time) : time(_time){}
	SimTime time;
};
/**
 * Typedef to allow us to use vectors in .msg files
 */
typedef std::vector<Identifier> PacketPath;
/*Vector of RouteStep-s that should represent route*/
typedef std::vector<RouteStep> RoutePath;

#endif //_TASK1_PROTOCOL_

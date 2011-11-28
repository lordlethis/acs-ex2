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
	bool operator <(const Identifier& other) const { return id < other.id; }
	bool operator >(const Identifier& other) const { return id > other.id; }
	bool operator <=(const Identifier& other) const { return id <= other.id; }
	bool operator >=(const Identifier& other) const { return id >= other.id; }
	operator t1id_t() const { return id; }
};

/**
 * Typedef to allow us to use vectors in .msg files
 */
typedef std::vector<Identifier> PacketPath;

class Payload {
public:
	virtual int getPayloadType() = 0;
};

class TicToc : public Payload {
public:
	TicToc(simtime_t _endTime) : endTime(_endTime) {}
	TicToc(const TicToc& other) : endTime(other.endTime) {}
	virtual int getPayloadType() { return 0; }
	simtime_t endTime;
};

class InitiateTicToc : public Payload {
public:
	InitiateTicToc(const Identifier& dst) : target(dst) {}
	InitiateTicToc() {}
	virtual int getPayloadType() { return 1; }
	Identifier target;
};

//typedef boost::shared_ptr<Payload> ObjectPtr; // this fails because of an ambiguous ostream overload :(

// so yeah, this introduces a memory leak. I don't care anymore at this point, only want to get the exercise done. :(
// TODO: ask what the best way of embedding a payload in packets would be...
typedef Payload* ObjectPtr;

#endif //_TASK1_PROTOCOL_

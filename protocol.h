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

/**
 * Typedef to allow us to use vectors in .msg files
 */
typedef std::vector<Identifier> PacketPath;

#endif //_TASK1_PROTOCOL_

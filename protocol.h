#ifndef _TASK1_PROTOCOL_
#define _TASK1_PROTOCOL_

#include <vector>
#include <omnetpp.h>
#include <sstream>

/**
 * Stupid wrapper class around identifiers. Initially implemented to allow using __int128,
 * and left in due to laziness... ^.^
 */
struct Identifier : public cObject
{
	Identifier() : x(0),y(0) {}
	Identifier(const Identifier& other) : x(other.x),y(other.y) {}
	Identifier(int _x, int _y) : x(_x), y(_y) {}
	virtual std::string info() const { std::stringstream out; out << "(" << x << " / " << y << ")"; return out.str(); }
	virtual std::string detailedInfo() const { return info(); }
	int x;
	int y;
	bool operator ==(const Identifier& other) const { return x == other.x && y == other.y; }
};

class Payload
{
public:
	virtual int getPayloadType() = 0;
};

class TicTocPayload : public Payload
{
public:
	virtual int getPayloadType() { return 0; }
	simtime_t endTime;
};

/**
 * Typedef to allow us to use vectors in .msg files
 */
typedef std::vector<Identifier> PacketPath;
typedef Payload* PacketPayload;

#endif //_TASK1_PROTOCOL_

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
	virtual int distance(const Identifier& other) const { int dx = x-other.x; int dy = y-other.y; return dx*dx+dy*dy; }
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
	TicTocPayload() : counter(0) {}
	virtual int getPayloadType() { return 0; }
	simtime_t endTime;
	int counter;
	bool doRecord;
};
class InitTicTocPayload : public Payload
{
public:
	virtual int getPayloadType() { return 1; }
	Identifier destination;
	bool doRecord;
};

/**
 * Typedef to allow us to use vectors in .msg files
 */
typedef std::vector<Identifier> PacketPath;
typedef Payload* PacketPayload;

#endif //_TASK1_PROTOCOL_

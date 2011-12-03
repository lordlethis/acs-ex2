#ifndef _TASK1_PROTOCOL_
#define _TASK1_PROTOCOL_

#include <map>
#include <omnetpp.h>
#include <sstream>


/**
 * Stupid wrapper class around identifiers. Initially implemented to allow using __int128,
 * and left in due to laziness... ^.^
 */
class Identifier : public cObject
{
public:
	Identifier() {}
	Identifier(const Identifier& other) : ids(other.ids) {}
	virtual std::string info() const { std::stringstream out; out << "("; unsigned int n = 1; for (IdentMap::const_iterator i = ids.begin(); i != ids.end(); ++i) out << i->second << (n++ < ids.size() ? ", " : ""); out << ")"; return out.str(); }
	virtual std::string detailedInfo() const { return info(); }
	bool operator ==(const Identifier& other) const { return other.ids == ids; }
	typedef std::map<int, int> IdentMap;
	bool hasLandmarkId(const int lmId) const { return ids.find(lmId) != ids.end(); } // do we have a landmark id?
	int& operator[](const int& lmId) { return ids[lmId]; } // get/set hop count for lmId
	const IdentMap& getIds() const { return ids; }
private:
	IdentMap ids;
};

class Payload {
public:
	virtual int getPayloadType() = 0;
};
struct TicTocPayload : public Payload {
	virtual int getPayloadType() { return 0; }
	simtime_t endTime;
};


/**
 * Typedef to allow us to use vectors in .msg files
 */
typedef std::vector<Identifier> PacketPath;
typedef Payload* PacketPayload;

#endif //_TASK1_PROTOCOL_

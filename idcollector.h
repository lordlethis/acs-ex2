
#ifndef IDCOLLECTOR_H_
#define IDCOLLECTOR_H_

#include <vector>
#include <utility>
#include <algorithm>
#include <map>
#include "protocol.h"

class IdCollector
{
public:
	static IdCollector* instance();
	void addId(const Identifier& id) { ids.push_back(id); }
	void removeId(const Identifier& id) { ids.erase(std::find(ids.begin(),ids.end(),id)); }
	const std::vector<Identifier> getIds() const { return ids; }
	std::vector<PacketPath> communicationPaths;
	std::vector<std::pair<std::pair<Identifier,Identifier>, int > > failedConnections;
	typedef std::vector<PacketPath> CommunicationPath;
	typedef std::vector<std::pair<std::pair<Identifier,Identifier>, int > > FailedConnectionList;
private:
	IdCollector() {}
	IdCollector(const IdCollector& idc) {}
	std::vector<Identifier> ids;

	static IdCollector* idc;
};

IdCollector* IdCollector::idc = new IdCollector();

IdCollector* IdCollector::instance()
{
	if (!idc) idc = new IdCollector();
	return idc;
}

#endif // IDCOLLECTOR_H_

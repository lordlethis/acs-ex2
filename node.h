//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef TASK2NODE_H_
#define TASK2NODE_H_

#include "commonnode.h"
//#include <omnetpp.h>

class Identifier;

/**
 * A class to hold the code of nodes o.o
 */
class IdNode : public CommonNode {
public:
	IdNode() {}
	virtual ~IdNode() {}
protected:
  // The following redefined virtual function holds the algorithm.
  virtual void initialize();
 // virtual void handleMessage(cMessage *msg);
  /** set hasId and set node color in ui accordingly */
  virtual void setHasId(bool has);
  virtual bool hasId() const { return _hasId; }
  virtual Identifier* getId() { return id; }
  virtual Identifier* getId() const { return id; }
  virtual void scheduleHeartBeatCheck();
  virtual HandlingState handleUncommonMessage(cMessage *msg);
  virtual bool handleSelfMessage(cMessage *msg);
private:
  cEnvir& log();
  bool _hasId;
  int initialDelay;
  int rejoinDelay;
  int retryTime;
  int minKeepIdTime;
  int maxKeepIdTime;
  long acquireMessageId;
  /** sequence number of the last heart beat msg we received */
  long prevBeatSeq;
  /** time when we received the last heart beaet */
  simtime_t prevBeatTime;
  /** did we already receive a heart beat earlier on? */
  bool firstBeat;
  /** the interval in which the server is firing heart beats */
  double beatInterval;
  Identifier* id;
  /** self-message to instruct ourselves to drop out of the network */
  cMessage *dropout;
};

#endif /* TASK2NODE_H_ */

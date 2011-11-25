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

#include <omnetpp.h>

class Identifier;

class Task2Node : public cSimpleModule {
public:
	Task2Node() {}
	virtual ~Task2Node() {}
protected:
  // The following redefined virtual function holds the algorithm.
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
  /** @since task 3 - code deduplication w/o backport to task 2 project... */
  virtual void setHasId(bool has);
  /** @since task 3 */
  virtual void scheduleHeartBeatCheck();
private:
  cEnvir& log();
  bool hasId;
  int initialDelay;
  int rejoinDelay;
  int retryTime;
  int minKeepIdTime;
  int maxKeepIdTime;
  long acquireMessageId;
  /** @since task 3 */
  long prevBeatSeq;
  /** @since task 3 */
  simtime_t prevBeatTime;
  /** @since task 3 */
  bool firstBeat;
  /** @since task 3 */
  int beatInterval;
  Identifier* id;
  /** @since task 3 */
  cMessage *dropout;
};

#endif /* TASK2NODE_H_ */

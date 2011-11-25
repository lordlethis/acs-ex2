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

enum MessageType {
    PING = 1;
    PONG = 2;
    ACQUIRE_ID = 3;
    ID_ASSIGNMENT = 4;
    HEARTBEAT = 5;
}

cplusplus {{
#include "protocol.h"
}}
struct Identifier;
struct PacketPath;

message AcsMessage {
    int msgType;
    PacketPath path;
}

message Ping extends AcsMessage {
    msgType = PING;
    Identifier id;
}
message Pong extends AcsMessage {
    msgType = PONG;
	Identifier id;
}

message AcquireId extends AcsMessage {
    msgType = ACQUIRE_ID;
    Identifier id;
    bool hasId;
    long messageId;
}

message IdAssignment extends AcsMessage {
    msgType = ID_ASSIGNMENT;
    Identifier id;
    long messageId;
    long lastHeartBeat;
    int beatInterval;
}

// added in task 3
message HeartBeat extends AcsMessage {
    msgType = HEARTBEAT;
    long seq;
}

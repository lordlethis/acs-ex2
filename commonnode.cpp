/*
 * commonnode.cpp
 *
 *  Created on: Nov 26, 2011
 *      Author: kei
 */

#include "commonnode.h"
#include "types.h"
#include "protocol_m.h"

CommonNode::CommonNode() {
}

CommonNode::~CommonNode() {
}

void CommonNode::handleMessage(cMessage *msg)
{
	if (msg->isSelfMessage())
	{
		handleSelfMessage(msg);
	}
	else
	{
		// short check for a cycling packet (removing such abominations)
		AcsMessage* amsg = dynamic_cast<AcsMessage*>(msg);
		if (amsg)
		{
			if (hasId())
			{
				for (size_t i = 0; i < amsg->getPath().size(); ++i)
				{
					if (amsg->getPath()[i] == *getId())
					{
						delete msg;
						return;
					}
				}
			}
		}
		HandlingState h = handleUncommonMessage(msg);
		if (!(h & HandlingStates::HANDLED))
		{
			h |= handleCommonMessage(msg);
		}
		if (amsg && (h & HandlingStates::FORWARD))
		{
			// broadcast code
			if (hasId())
			{
				// send a copy of the message on all gates except for the input gate of the message
				int idx = amsg->getArrivalGate()->getIndex();
				amsg->getPath().push_back(*getId());
				for (int i = 0; i < gateSize("gate"); ++i) {
					if (i == idx) continue;
					AcsMessage *m = amsg->dup();
					if (m)
					{
						send(m, "gate$o",i);
					}
				}
			}
		}
		if (!(h & HandlingStates::NODELETE))
			delete msg;
	}
}

CommonNode::HandlingState CommonNode::handleCommonMessage(cMessage* msg)
{
	AcsMessage* amsg = dynamic_cast<AcsMessage*>(msg);
	CommonNode::HandlingState state = HandlingStates::UNHANDLED;
	switch (amsg->getMsgType())
	{
	case PING:
	{
		// ignore ping if we're not associated
		if (hasId())
		{
			if (((Ping*)amsg)->getId().id == getId()->id)
			{
				// we were pinged. respond with pong :D
				Pong* pong = new Pong("PONG");
				pong->setId(getId()->id);
				pong->getPath().push_back(*getId());
				cGate* gate = msg->getArrivalGate();
				send(pong,"gate$o",gate->getIndex());
				state = HandlingStates::HANDLED;
			}
			else
			{
				state = HandlingStates::FORWARD | HandlingStates::HANDLED;
			}
		}
		break;
	} // end PING
	case ACQUIRE_ID:
	case PONG:
	{
		state = HandlingStates::FORWARD;
		break;
	} // end PONG
	case ROUTE_REC:
	{
		if (hasId())
		{
			if (((RouteRecord*)amsg)->getDest().id != getId()->id) // tictoc msg is not for us
			{
				//we record route step an send it broadcast it further on
				RouteRecord* rmsg = dynamic_cast<RouteRecord*>(amsg);
				int idx = rmsg->getArrivalGate()->getIndex();
				rmsg->getPath().push_back(*getId());

				// new step has arrival gate and current node Id
				RouteStep *step = new RouteStep(getId()->id, idx);
				rmsg->getRoute().push_back(*step);
				for (int i = 0; i < gateSize("gate"); ++i) {
					if (i == idx) continue;
					AcsMessage *m = rmsg->dup();
					if (m)
					{
						send(m, "gate$o",i);
					}
				}

				state = HandlingStates::HANDLED;
			}
			else	// tictoc msg is for us so we send tic node that we confirm comunication
			{
				if(!playing)
				{
					EV << "I AM TOC: " << getId()->id << "\n";
					setDisplayString("i=block/routing,blue");
					playing=true;
					bubble("I AM TOC");
					// creating the statistics log
					timeVector.record((simTime() - ((RouteRecord*)amsg)->getStart().time)*2);
					timeVector.record(((RouteRecord*)amsg)->getPath().size());

					// we create routable message that leads to tic node
					Routable *Toc = new Routable("ROUTABLE",ROUTABLE);
					Toc->getPath().push_back(*getId());
					Toc->setSource(*getId());
					Toc->setDest(((RouteRecord*)amsg)->getSource().id);
					Toc->setRoute(((RouteRecord*)amsg)->getRoute()); // copy route that arrived
					Toc->setEnd(((RouteRecord*)amsg)->getEnd());
					Toc->setTictoc(1);	// mark type with 12 that means that next node that receives it is tic node
					cGate* gate = msg->getArrivalGate();
					send(Toc,"gate$o",gate->getIndex());
				}
				state = HandlingStates::HANDLED;
			}
		}
		break;
	}
	case ROUTABLE:
	{
		if(hasId())
		{
			if(((Routable*)amsg)->getDest().id == getId()->id)// tictoc message arrived to one of the nodes(tic or toc)
			{
				if(((Routable*)amsg)->getTictoc()==3)// signal for the toc to stop comunicating
				{
					setDisplayString("i=block/routing,green");
					bubble("LAST TOC!");
					playing=false;
					state = HandlingStates::HANDLED;
				}else
				{			// we make new message to send it back, continuing the tic-toc communication
					Routable* old = dynamic_cast<Routable*>(amsg);
					Routable* msg = new Routable("ROUTABLE",ROUTABLE);
					msg->getPath().push_back(*getId());

					msg->setSource(*getId());
					msg->setDest(old->getSource().id);
					msg->setRoute(old->getNewRoute());
					if(old->getTictoc()==1)	// we are tic
					{
						if(old->getEnd().time <= simTime())	// if session time has run out we send message flaged =3
						{									// to stop the communication
							playing=false;
							setDisplayString("i=block/routing,green");
							msg->setTictoc(3);
							bubble("LAST TIC!");
						}else
						{
							msg->setTictoc(2);	// set the flag for the next node to be toc
							bubble("TIC!");
						}
					}
					else	//we are toc
					{
						msg->setTictoc(1);	// set the flag for the next node to be tic
						bubble("TOC!");
					}
					msg->setEnd(old->getEnd());

					int gate = amsg->getArrivalGate()->getIndex();
					send(msg,"gate$o",gate);

					state = HandlingStates::HANDLED;

				}
			}else // tictoc message is not for us and we forward it to the next node in the route
			{
				EV << "FORWARDING ROUTABLE MSG.\n";
				Routable* rmsg = dynamic_cast<Routable*>(amsg);
				rmsg->getPath().push_back(*getId());

				// Recording new route
				int idx = rmsg->getArrivalGate()->getIndex();
				RouteStep *step = new RouteStep(getId()->id, idx);
				rmsg->getNewRoute().push_back(*step);

				// Using the last element in the route to get the next hop
				int gate = rmsg->getRoute().back().gateNum;

				// Removing the last element in the route to prepare it for the next node
				rmsg->getRoute().pop_back();

				AcsMessage *m = rmsg->dup();
				if (m)send(m,"gate$o",gate);

				state = HandlingStates::HANDLED;
			}
		}
		break;
	}
	} // end switch msgtype
	return state;
}

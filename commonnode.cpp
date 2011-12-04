/*
 * commonnode.cpp
 *
 *  Created on: Nov 26, 2011
 *      Author: kei
 */

#include "commonnode.h"
#include "protocol_m.h"
#include "idcollector.h"
#include <iostream>

#define ALL_IDS (IdCollector::instance()->getIds())

CommonNode::CommonNode() : initiateTicToc("SELFMSG_INITIATE_TICTOC"), sendHello("SEND_HELLO"),coordinator(false) {
}

CommonNode::~CommonNode() {
}

void CommonNode::initialize()
{
	tictocStart = par("tictocStart");
	tictocInterval = par("tictocInterval");
	helloInterval = par("helloInterval");
	int x = par("x");
	int y = par("y");
	_id = Identifier(x,y);
	IdCollector::instance()->addId(_id);
	if ((*IdCollector::instance()->getIds().begin())==getId())
	{
		scheduleAt(simTime()+tictocStart, &initiateTicToc);
		coordinator = true;
		ttStart = 0;
		ttIter = 0;
	}
	scheduleAt(simTime(), &sendHello);
	getDisplayString().setTagArg("t",0,getId().info().c_str());
}

void CommonNode::finish()
{
	if (coordinator)
	{
		// write statistics to stderr
		std::cerr << "# failed tictocs: " << IdCollector::instance()->failedConnections.size() << "\n";
		for (IdCollector::FailedConnectionList::iterator fiter = IdCollector::instance()->failedConnections.begin(); fiter != IdCollector::instance()->failedConnections.end(); fiter++)
		{
			std::cerr << "   " << fiter->first.first.info() << " <-> " << fiter->first.second.info() << " [" << fiter->second << "]" <<"\n";
		}
		std::cerr << "# recorded paths: " << IdCollector::instance()->communicationPaths.size() << "\n";
		for (IdCollector::CommunicationPath::iterator citer = IdCollector::instance()->communicationPaths.begin(); citer != IdCollector::instance()->communicationPaths.end(); citer++)
		{
			std::stringstream ss;
			for (PacketPath::iterator piter = citer->begin(); piter != citer->end();)
			{
				ss << piter->info();
				++piter;
				if (piter != citer->end())
					ss << " --> ";
			}
			std::cerr << "   " << ss.str() << "\n";
		}
	}
}

void CommonNode::handleMessage(cMessage *msg)
{
	if (msg->isSelfMessage())
	{
		handleSelfMessage(msg);
	}
	else
	{
		AcsMessage* amsg = dynamic_cast<AcsMessage*>(msg);
		HandlingState h = handleUncommonMessage(msg);
		if (!(h & HandlingStates::HANDLED))
		{
			h |= handleCommonMessage(msg);
		}
		if (amsg && (h & HandlingStates::BROADCAST))
		{
			// broadcast code
			if (hasId())
			{
				// send a copy of the message on all gates except for the input gate of the message
				int idx = amsg->getArrivalGate()->getIndex();
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

void CommonNode::handleSelfMessage(cMessage* msg)
{
	if (msg == &initiateTicToc)
	{
		// try and initiate a tictoc sequence
		int nnodes = ALL_IDS.size();
		if (ttIter==ttStart)
			++ttIter;
		if (ttIter >= nnodes)
		{
			ttIter = 0;
			++ttStart;
		}
		if (ttStart >= nnodes)
		{
			log() << "Performed tic toc on all pairs. I'm done!\n";
			return;
		}
		int n1 = ttStart;
		int n2 = ttIter;

		RoutableMessage* rmsg = new RoutableMessage("INIT_TICTOC");
		InitTicTocPayload *ittp = new InitTicTocPayload;
		ittp->destination = ALL_IDS[n2];
		ittp->doRecord = getRNG(0)->doubleRand()>0.75;
		rmsg->setTarget(ALL_IDS[n1]);
		rmsg->setSource(getId());
		rmsg->setPayload(ittp);
		log() << "Sending tictoc init msg for tictoc between " << rmsg->getTarget().info() << " <-> " << ittp->destination.info() << "\n";
		if (rmsg->getTarget() == getId())
			handleRoutableMessage(rmsg);
		else
			forwardMessage(rmsg);
		scheduleAt(simTime()+tictocInterval, msg);
		++ttIter;
	}
	else if (msg == &sendHello)
	{
		// send out hello messages so our neighbours know about us
		for (int i = 0; i < gateSize("gate"); ++i)
		{
			HelloMessage* hmsg = new HelloMessage("HELLO_THERE");
			hmsg->setSource(getId());
			send(hmsg, "gate$o", i);
		}
		scheduleAt(simTime()+helloInterval,msg);
	}
}

CommonNode::HandlingState CommonNode::handleCommonMessage(cMessage* msg)
{
	AcsMessage* amsg = dynamic_cast<AcsMessage*>(msg);
	CommonNode::HandlingState state = HandlingStates::UNHANDLED;
	switch (amsg->getMsgType())
	{
	case HELLO:
	{
		HelloMessage* hmsg = (HelloMessage*)amsg;
		Identifier source = hmsg->getSource();
		// remember where the neighbour lives :-)
		neighbours[hmsg->getArrivalGate()->getIndex()] = source;
		break;
	}
	case ROUTABLE:
	{
		// handle the message if it is for us
		// or just try to forward it.
		RoutableMessage* rmsg = (RoutableMessage*)amsg;
		if (rmsg->getTarget() == getId())
		{
			handleRoutableMessage(rmsg);
		}
		else
		{
			forwardMessage(rmsg);
		}
		state = HandlingStates::HANDLED | HandlingStates::NODELETE;
	}
	default:
		break;
	} // end switch msgtype
	return state;
}

void CommonNode::handleRoutableMessage(RoutableMessage *msg)
{
	if (!msg->getPayload())
		return;
	switch (msg->getPayload()->getPayloadType())
	{
	case 0: // TICTOC
	{
		TicTocPayload* tt = (TicTocPayload*)msg->getPayload();
		// check if the tictoc should go on - if not, stop it by deleting the message
		if (tt->endTime <= simTime())
		{
			log() << "Tictoc should end... Byebye... :)" << tt->endTime << " / cur: " << simTime() << "\n";
			delete tt;
			delete msg;
			return;
		}
		// collect some stats (maybe)
		msg->getPath().push_back(getId());
		tt->counter++;
		if (tt->doRecord && (tt->counter == 2 || tt->counter == 3) && IdCollector::instance()->communicationPaths.size()<30)
		{
			IdCollector::instance()->communicationPaths.push_back(msg->getPath());
		}
		// reset the path vector, switch source/target, send it back out
		msg->setTarget(msg->getSource());
		msg->setSource(getId());
		msg->getPath().clear();
		forwardMessage(msg);
	}
	break;
	case 1: // init tictoc
	{
		// initiate the tic toc message...
		InitTicTocPayload* ittp = (InitTicTocPayload*)msg->getPayload();
		TicTocPayload *ttp = new TicTocPayload;
		RoutableMessage *rmsg = new RoutableMessage("Tic toc goes the clock...");
		ttp->endTime = simTime()+getRNG(0)->doubleRandIncl1()*30.0+30.0;
		ttp->doRecord = ittp->doRecord;
		rmsg->setPayload(ttp);
		rmsg->setTarget(ittp->destination);
		rmsg->setSource(getId());
		forwardMessage(rmsg);
		delete ittp;
		delete msg;
	}
	break;
	default:
		log() << "unknown payload type " << msg->getPayload()->getPayloadType() << "\n";
	}
}

static void noop() {}

void CommonNode::forwardMessage(RoutableMessage* msg)
{
	int gateNum = -1;
	int minDist = msg->getTarget().distance(getId());
	msg->getPath().push_back(getId());
	if (msg->getSource().x == 1 && msg->getSource().y == 0 && msg->getTarget().x == 0 && msg->getTarget().y == 2)
	{
		noop(); // for debug breakpoints
	}
	// greedily decide on next hop, if any
	for (NeighbourList::iterator iter = neighbours.begin(); iter!=neighbours.end(); ++iter)
	{
		int distance = iter->second.distance(msg->getTarget());
		if (distance < minDist)
		{
			minDist = distance;
			gateNum = iter->first;
		}
	}
	// either send the message through the greedily determined gate, or drop it
	if (gateNum != -1)
		send(msg,"gate$o",gateNum);
	else
	{
		if (msg->getPayload()->getPayloadType()==0)
		{
			IdCollector::instance()->failedConnections.push_back(std::pair<std::pair<Identifier,Identifier>,int >(std::pair<Identifier,Identifier>(msg->getSource(), msg->getTarget()),((TicTocPayload*)msg->getPayload())->counter));
			std::cerr << "dropping message at " << getId().info() << ": " << msg->getSource().info() << "<-->" << msg->getTarget().info() << "\n";
		}
		log() << "Dropping routable message - there's no neighbour that is closer to the target than we are already. Greedy routing fails." << "\n";
	}
}

cEnvir& CommonNode::log()
{
	return ( EV << "[" << getName() << " - " << getId().info() << "] " );
}

/*
 * NetworkNode.cpp
 *
 *  Created on: Dec 2, 2011
 *      Author: kei
 */

#include "networknode.h"

NetworkNode::NetworkNode() : CommonNode() {
}

NetworkNode::~NetworkNode() {
}

// Register module class with OMNeT++
Define_Module(NetworkNode);

void NetworkNode::handleSelfMessage(cMessage* msg)
{
}

package exercise2;

import ned.DelayChannel;


@license(LGPL);


simple IdServer
{
    parameters:
        @display("i=block/routing,gold");
        @display("p=200,200");
        int timeout = default(2);
        int range_start = default(0);
        int range_end = default(2147483647);
        int pulse_rate = default(0); // original: 1
        int helloInterval = default(5.5);
        int routingTableTimeout = default(6);
        int ticTocInterval = default(60);
        int ticTocStartTime = default(180);
        int ticTocDuration = default(180);
    gates:
        inout gate[];
}

simple IdNode
{
    parameters:
        @display("i=block/routing,gray");
        @display("p=50,50,ring,150,150");
        int initialDelay = default(-1); // when the node will join the net (-1 means random time)
        int minKeepIdTime = default(30); // minKeepIdTime
        int maxKeepIdTime = default(0); //
        int rejoinDelay = default(5); // 
        int retryTime = default(10); //
        int helloInterval = default(5);
        int routingTableTimeout = default(6);
        int ticTocDuration = default(180);
    gates:
        inout gate[];
}

network Network
{
    parameters:
        int numClients @prompt("Number of clients") = default(4);
    submodules:
        server: IdServer;
        clients[numClients+3]: IdNode;
    connections allowunconnected:
        for i=0..numClients-1 {
            server.gate++ <--> {  delay = 10ms; } <--> clients[i].gate++;
            clients[i].gate++ <--> {  delay = 10ms; } <--> clients[i+1].gate++;
        }
        clients[numClients].gate++ <--> {  delay = 10ms; } <--> clients[0].gate++;
        clients[numClients+1].gate++ <--> {  delay = 10ms; } <--> clients[numClients].gate++;
        clients[numClients+2].gate++ <--> {  delay = 10ms; } <--> clients[numClients+1].gate++;
        clients[numClients+2].gate++ <--> {  delay = 10ms; } <--> clients[0].gate++;
}
//
// TODO documentation
//
// @author kei
//
network Network1
{
    @display("bgb=791,421");
    submodules:
        idServer: IdServer {
            @display("p=313,185");
        }
        idNode: IdNode {
            @display("p=463,185");
        }
        idNode1: IdNode {
            @display("p=436,81");
        }
        idNode2: IdNode {
            @display("p=250,74");
        }
        idNode3: IdNode {
            @display("p=155,140");
        }
        idNode4: IdNode {
            @display("p=175,306");
        }
        idNode5: IdNode {
            @display("p=397,339");
        }
        idNode6: IdNode {
            @display("p=529,303");
        }
        idNode7: IdNode {
            @display("p=287,296");
        }
        idNode8: IdNode {
            @display("p=73,230");
        }
        idNode9: IdNode {
            @display("p=73,78");
        }
        idNode10: IdNode {
            @display("p=575,136");
        }
    connections:
        idServer.gate++ <--> {  delay = 10ms; } <--> idNode2.gate++;
        idServer.gate++ <--> {  delay = 10ms; } <--> idNode.gate++;
        idServer.gate++ <--> {  delay = 10ms; } <--> idNode7.gate++;
        idNode7.gate++ <--> {  delay = 10ms; } <--> idNode5.gate++;
        idNode5.gate++ <--> {  delay = 10ms; } <--> idNode.gate++;
        idNode6.gate++ <--> {  delay = 10ms; } <--> idNode10.gate++;
        idNode.gate++ <--> {  delay = 10ms; } <--> idNode6.gate++;
        idNode2.gate++ <--> {  delay = 10ms; } <--> idNode1.gate++;
        idNode5.gate++ <--> {  delay = 10ms; } <--> idNode1.gate++;
        idNode.gate++ <--> {  delay = 10ms; } <--> idNode1.gate++;
        idNode4.gate++ <--> {  delay = 10ms; } <--> idServer.gate++;
        idNode3.gate++ <--> {  delay = 10ms; } <--> idNode7.gate++;
        idNode8.gate++ <--> {  delay = 10ms; } <--> idNode3.gate++;
        idNode9.gate++ <--> {  delay = 10ms; } <--> idNode2.gate++;
        idNode9.gate++ <--> {  delay = 10ms; } <--> idNode3.gate++;
        idNode9.gate++ <--> {  delay = 10ms; } <--> idNode4.gate++;
        idNode8.gate++ <--> {  delay = 10ms; } <--> idNode7.gate++;
        idNode10.gate++ <--> {  delay = 10ms; } <--> idNode5.gate++;
}


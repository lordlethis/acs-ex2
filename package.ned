package exercise2;

@license(LGPL);

simple IdServer
{
    parameters:
        @display("i=block/routing,gold");
        @display("p=200,200");
        int timeout = default(2000);
        int range_start = default(0);
        int range_end = default(2147483647);
        int pulse_rate = default(1000);
    gates:
        inout gate[];
}

simple IdNode
{
    parameters:
        @display("i=block/routing,gray");
        @display("p=50,50,ring,150,150");
        int initialDelay = default(-1); // when the node will join the net (-1 means random time)
        int minKeepIdTime = default(30000); // minKeepIdTime
        int maxKeepIdTime = default(40000); //
        int rejoinDelay = default(5000); // 
        int retryTime = default(10000); //
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
            clients[i].gate++ <--> { delay = 10ms; } <--> clients[i+1].gate++;
        }
        clients[numClients].gate++ <--> { delay = 10ms; } <--> clients[0].gate++;
        clients[numClients+1].gate++ <--> { delay = 10ms; } <--> clients[numClients].gate++;
        clients[numClients+2].gate++ <--> { delay = 10ms; } <--> clients[numClients+1].gate++;
        clients[numClients+2].gate++ <--> { delay = 10ms; } <--> clients[0].gate++;
}
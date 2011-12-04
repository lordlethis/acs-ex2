package exercise2;

import ned.DelayChannel;


@license(LGPL);

simple GpsNode
{
    parameters:
        @display("i=block/routing,gray");
        int x;
        int y;
        double tictocStart = default(60.1);
        double tictocInterval = default(60);
        double helloInterval = default(1);
    gates:
        inout gate[];
}

//
// TODO documentation
//
// @author kei
//
network Network
{
    @display("bgb=638,391");
    submodules:
        gpsNode: GpsNode {
            @display("p=53,306");
            x = 0;
            y = 0;
        }
        gpsNode1: GpsNode {
            @display("p=183,334");
            x = 1;
            y = 0;
        }
        gpsNode2: GpsNode {
            @display("p=262,309");
            x = 2;
            y = 0;
        }
        gpsNode3: GpsNode {
            @display("p=369,332");
            x = 3;
            y = 0;
        }
        gpsNode4: GpsNode {
            @display("p=482,246");
            x = 4;
            y = 1;
        }
        gpsNode5: GpsNode {
            @display("p=361,205");
            x = 3;
            y = 2;
        }
        gpsNode6: GpsNode {
            @display("p=196,241");
            x = 1;
            y = 1;
        }
        gpsNode7: GpsNode {
            @display("p=99,177");
            x = 0;
            y = 2;
        }
        gpsNode8: GpsNode {
            @display("p=175,129");
            x = 1;
            y = 3;
        }
        gpsNode9: GpsNode {
            @display("p=284,158");
            x = 2;
            y = 3;
        }
        gpsNode10: GpsNode {
            @display("p=394,109");
            x = 3;
            y = 3;
        }
        gpsNode11: GpsNode {
            @display("p=502,146");
            x = 4;
            y = 3;
        }
        gpsNode12: GpsNode {
            @display("p=33,96");
            x = 0;
            y = 3;
        }
    connections:
        gpsNode12.gate++ <--> { delay = 10ms; } <--> gpsNode8.gate++;
        gpsNode12.gate++ <--> { delay = 10ms; } <--> gpsNode.gate++;
        gpsNode.gate++ <--> { delay = 10ms; } <--> gpsNode1.gate++;
        gpsNode1.gate++ <--> { delay = 10ms; } <--> gpsNode2.gate++;
        gpsNode2.gate++ <--> { delay = 10ms; } <--> gpsNode3.gate++;
        gpsNode3.gate++ <--> { delay = 10ms; } <--> gpsNode5.gate++;
        gpsNode9.gate++ <--> { delay = 10ms; } <--> gpsNode5.gate++;
        gpsNode5.gate++ <--> { delay = 10ms; } <--> gpsNode10.gate++;
        gpsNode10.gate++ <--> { delay = 10ms; } <--> gpsNode11.gate++;
        gpsNode11.gate++ <--> { delay = 10ms; } <--> gpsNode4.gate++;
        gpsNode4.gate++ <--> { delay = 10ms; } <--> gpsNode3.gate++;
        gpsNode4.gate++ <--> { delay = 10ms; } <--> gpsNode5.gate++;
        gpsNode9.gate++ <--> { delay = 10ms; } <--> gpsNode2.gate++;
        gpsNode5.gate++ <--> { delay = 10ms; } <--> gpsNode6.gate++;
        gpsNode6.gate++ <--> { delay = 10ms; } <--> gpsNode.gate++;
        gpsNode7.gate++ <--> { delay = 10ms; } <--> gpsNode.gate++;
        gpsNode7.gate++ <--> { delay = 10ms; } <--> gpsNode8.gate++;
        gpsNode7.gate++ <--> { delay = 10ms; } <--> gpsNode9.gate++;
        gpsNode9.gate++ <--> { delay = 10ms; } <--> gpsNode8.gate++;
        gpsNode8.gate++ <--> { delay = 10ms; } <--> gpsNode6.gate++;
        gpsNode1.gate++ <--> { delay = 10ms; } <--> gpsNode6.gate++;
}


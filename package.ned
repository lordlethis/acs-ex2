package exercise2;

import ned.IdealChannel;
import ned.IBidirectionalChannel;


@license(LGPL);

simple LandmarkNode
{
    parameters:
        @display("i=block/routing,green");
        int landmarkId;
    gates:
        inout gate[];
}

simple NetworkNode
{
    parameters:
        @display("i=block/routing,gray");
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
    @display("bgb=744,602");
    submodules:
        lm1: LandmarkNode {
            @display("p=115,106");
            landmarkId = 1;
        }
        lm2: LandmarkNode {
            @display("p=373,221");
            landmarkId = 2;
        }
        lm3: LandmarkNode {
            @display("p=379,42");
            landmarkId = 3;
        }
        networkNode: NetworkNode {
            @display("p=102,234");
        }
        networkNode1: NetworkNode {
            @display("p=261,75");
        }
        networkNode2: NetworkNode {
            @display("p=215,188");
        }
        networkNode3: NetworkNode {
            @display("p=254,311");
        }
        networkNode4: NetworkNode {
            @display("p=128,333");
        }
        networkNode5: NetworkNode {
            @display("p=370,391");
        }
        networkNode6: NetworkNode {
            @display("p=241,419");
        }
        networkNode7: NetworkNode {
            @display("p=474,133");
        }
        networkNode8: NetworkNode {
            @display("p=566,205");
        }
        networkNode9: NetworkNode {
            @display("p=475,283");
        }
    connections:
        lm1.gate++ <--> IdealChannel <--> networkNode1.gate++;
        networkNode1.gate++ <--> IdealChannel <--> lm2.gate++;
        networkNode1.gate++ <--> IdealChannel <--> lm3.gate++;
        lm3.gate++ <--> IdealChannel <--> networkNode7.gate++;
        networkNode7.gate++ <--> IdealChannel <--> lm2.gate++;
        networkNode7.gate++ <--> IdealChannel <--> networkNode9.gate++;
        networkNode9.gate++ <--> IdealChannel <--> lm2.gate++;
        networkNode9.gate++ <--> IdealChannel <--> networkNode8.gate++;
        lm2.gate++ <--> IdealChannel <--> networkNode3.gate++;
        networkNode2.gate++ <--> IdealChannel <--> networkNode3.gate++;
        networkNode2.gate++ <--> IdealChannel <--> networkNode5.gate++;
        lm1.gate++ <--> IdealChannel <--> networkNode.gate++;
        networkNode.gate++ <--> IdealChannel <--> networkNode2.gate++;
        networkNode3.gate++ <--> IdealChannel <--> networkNode6.gate++;
        networkNode6.gate++ <--> IdealChannel <--> networkNode5.gate++;
        networkNode3.gate++ <--> IdealChannel <--> networkNode4.gate++;
        networkNode2.gate++ <--> IdealChannel <--> networkNode4.gate++;
}


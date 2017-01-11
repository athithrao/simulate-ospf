//
// Created by Athith Amarnath on 12/4/16.
//

#ifndef OSPF_ROUTER_H
#define OSPF_ROUTER_H
//
// Created by aldrichj on 11/10/16.
//

#ifndef CS457PROJECT3_ROUTER_H
#define CS457PROJECT3_ROUTER_H

#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <csignal>
#include <string.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <ctime>
#include <sys/select.h>
#include <atomic>
#include "logger.h"
#include "Graph.h"

using namespace std;

#define MAXROUTERS 25

class Router {

    struct neighborData {
        int neighborID;
        int neighborCost;
        uint16_t neighborPort;
    };

    struct routingTable {
        int sourceId;
        int destId;
        int cost;
        uint16_t destPort;
    };


public:
    void parseRouterData(stringstream& in);
    void routerProcess(char *argv);
    void sendUDPPacket(stringstream& in, int destId);
    void sendTCPPacket(stringstream& in);
    stringstream& receiveUDPPacket(stringstream& in);
    stringstream& receiveTCPPacket(stringstream& in);
    bool checklimitedBroadcastStatus();
    void closeRouterPort();
    uint16_t getdestRouterPort(int destId);
    stringstream& formUDPBroadcastData(stringstream& in);
    void parseUDPBroadcastData(stringstream& in);
    void removeDuplicateEntry();
    void calcshortpath();
    int checkPacketData(stringstream& in);
    void sendUDPBroadcastData(stringstream &in);

private:
    FileLogger *myLog;
    int routerNumber;
    int totalRouters;
    int totalRouterDataReceived[MAXROUTERS];
    uint16_t TCPPortno;
    uint16_t UDPPortno;
    uint16_t receivedUDPPortno;
    int TCPsocket;
    int UDPsocket;
    //std::atomic<int> control_flags[2] = {}; // control flags: | router contact neighbors | all routers acked |
    vector<neighborData> neighbors;
    vector<routingTable> routerTable;
    Graph *graph;
    int controlFlag;
    int LBReadyFlag;

};


#endif //CS457PROJECT3_ROUTER_H

#endif //OSPF_ROUTER_H

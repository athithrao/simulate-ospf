//
// Created by aldrichj on 11/10/16.
//

#ifndef CS457PROJECT3_MANAGER_H
#define CS457PROJECT3_MANAGER_H


#include <fstream>
#include <vector>
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include "logger.h"

using namespace std;

#define MAXROUTERS 25

class Manager
{




public:
        void createRouters();
        void serverTCP();
        stringstream& formRouterData(stringstream &data, int NodeNo);
        void sendRouterInfo(int control_status);
        void readTopologyData(char* fileName);
        bool areAllRoutersConnected(int control_status);
        std::thread createRoutersThread() {
            return std::thread([=] { createRouters(); });
        }
        std::thread serverTCPThread() {
            return std::thread([=] { serverTCP(); });
        }
        stringstream& formPacketData(stringstream &data, int packetnum);
        void destroyRouters();

private:
        struct packetInfo
        {
            int src;
            int dest;
            bool status;
        };
        ifstream routerFile;
        pid_t children[MAXROUTERS];
        std::vector<packetInfo> packetData;
        FileLogger *myLog;
        string *routerTopologyData;
        int topologyDatacount;
        uint16_t totalRouters;
        uint16_t routerUDPPorts[MAXROUTERS];
        uint16_t routerACKStatus[MAXROUTERS];
        uint16_t routerLBStatus[MAXROUTERS];
        uint16_t routerDAStatus[MAXROUTERS];
        int DARAck;
        int currPacketData;
        int quitAck;
};


#endif //CS457PROJECT3_MANAGER_H

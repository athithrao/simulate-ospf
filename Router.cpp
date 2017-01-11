//
// Created by Athith Amarnath on 12/4/16.
//

#include "Router.h"

void sig_handler(int);

Router a;

volatile sig_atomic_t stop;

size_t sizeofstringstream (stringstream &data)
{
    data.seekp(0, ios::end);
    stringstream::pos_type offset = data.tellp();
    return (size_t) offset;
}

void Router::parseRouterData(stringstream& in) {
    neighbors.clear();
    char delimiter, logdata[100];
    uint16_t temp;
    string info;

    size_t loc;
    neighborData n;
    routingTable t;

    in >> temp;
    routerNumber = ntohs(temp);
    in >> delimiter;

    in >> temp;
    totalRouters = ntohs(temp);
    in >> delimiter;

    graph = new Graph(totalRouters);

    controlFlag = 0;

    while (getline(in, info, '|')) {
        temp = atoi(info.substr(0, (info.find(" "))).c_str());
        loc = info.find_first_of(" ");
        info = info.substr(loc + 1);
        n.neighborID = atoi(info.substr(0, (info.find(" "))).c_str());
        t.sourceId = routerNumber;
        t.destId = n.neighborID;

        loc = info.find_first_of(" ");
        info = info.substr(loc + 1);
        n.neighborCost = atoi(info.substr(0, (info.find(" "))).c_str());
        t.cost = n.neighborCost;

        loc = info.find_first_of(" ");
        info = info.substr(loc + 1);
        n.neighborPort = ntohs(atoi(info.c_str()));
        t.destPort = n.neighborPort;
        neighbors.push_back(n);
        routerTable.push_back(t);
    }

    sprintf(logdata, "Router No - %d", routerNumber);
    *myLog << FileLogger::e_logType::LOG_INFO << logdata;
    sprintf(logdata, "Total Routers - %d", totalRouters);
    *myLog << FileLogger::e_logType::LOG_INFO << logdata;
    *myLog << FileLogger::e_logType::LOG_INFO << "Following are the immediate neighbours.";
    for (int i = 0; i < neighbors.size(); i++) {
        sprintf(logdata, "Neighbor ID - %d   Neighbor Cost - %d   Neighbor Port - %d", neighbors[i].neighborID,
                neighbors[i].neighborCost, neighbors[i].neighborPort);
        *myLog << FileLogger::e_logType::LOG_INFO << logdata;
    }
    for(int i = 0; i< MAXROUTERS; i++)
    {
        totalRouterDataReceived[i] = 0;
    }
    receivedUDPPortno = 0;
    LBReadyFlag = 0;
}

void Router::routerProcess(char *argv)
{
    struct sockaddr_in address;
    int status = 0, nextHop = 0;
    string ip = "127.0.0.1";
    stringstream TCPsend, TCPrecv, UDPsend, UDPrecv;
    stringstream route;
    char logdata[150];
    TCPPortno = 15000;
    UDPPortno = atoi(argv);

    int max_sd;
    fd_set readfds;

    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip.c_str(),&address.sin_addr.s_addr);
    address.sin_port = htons(TCPPortno);
    memset(address.sin_zero,0,sizeof(address.sin_zero));

    TCPsocket = socket(AF_INET, SOCK_STREAM,0);
    UDPsocket = socket(AF_INET, SOCK_DGRAM,0);
    int n = 1024 * 1024;
    setsockopt(UDPsocket, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));

    if(TCPsocket < 0)
    {
        cout<<"Error: Socket Establishment. Quitting Program.. \n";
        exit(1);
    }

    sockaddr_in sin;

    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family= AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = UDPPortno;

    while(::bind(UDPsocket, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        sin.sin_port = ++UDPPortno;
    }

    sprintf(logdata, "%d", UDPPortno);
    myLog = new FileLogger(0,logdata, strcat(logdata,".out"));

    *myLog << FileLogger::e_logType::LOG_INFO << "TCP and UDP Sockets created.";
    sprintf(logdata, "TCP Port No - %d", TCPPortno);
    *myLog << FileLogger::e_logType::LOG_INFO <<logdata;
    sprintf(logdata, "UDP Port No - %d", UDPPortno);
    *myLog << FileLogger::e_logType::LOG_INFO <<logdata;
    *myLog << FileLogger::e_logType::LOG_INFO << "Waiting to connect to the manager...";

    do
    {
        status = connect(TCPsocket,(struct sockaddr*)&address,sizeof(address));

    }while(status < 0);

    TCPsend <<'R'<< htons(UDPPortno);

    sendTCPPacket(TCPsend);
    TCPsend.str("");



    while (!stop)
    {
        signal(SIGINT, sig_handler);
        FD_ZERO(&readfds);

        FD_SET(UDPsocket, &readfds);
        FD_SET(TCPsocket, &readfds);

        max_sd = TCPsocket;

        if ( UDPsocket > max_sd)
                max_sd = UDPsocket;

        status = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((status < 0) && (errno != EINTR)) {
            cout << "Select function error. Quitting Program..\n";
            exit(1);
        }

        if (FD_ISSET(UDPsocket, &readfds))
        {
            if(controlFlag == 1)
            {
                UDPrecv.clear();
                UDPrecv.str("");
                receiveUDPPacket(UDPrecv);
                if(!checklimitedBroadcastStatus())
                    parseUDPBroadcastData(UDPrecv);
                UDPsend.clear();
                UDPsend.str("");
                UDPsend.str(UDPrecv.str());
                sendUDPBroadcastData(UDPsend);
            }
            else if (controlFlag == 3)
            {
                UDPrecv.clear();
                receiveUDPPacket(UDPrecv);//send receive packets.
                UDPsend.clear();
                UDPsend.str(UDPrecv.str());
                nextHop  = checkPacketData(UDPsend);
                if(nextHop!=-1)
                    sendUDPPacket(UDPsend, nextHop);
            }


        }

        if (FD_ISSET(TCPsocket, &readfds))
        {
            if(controlFlag == 0)
            {
                TCPrecv.clear();
                receiveTCPPacket(TCPrecv);
                parseRouterData(TCPrecv);
                TCPsend.str("READY");
                sendTCPPacket(TCPsend);
                TCPrecv.clear();
                receiveTCPPacket(TCPrecv);
                for(int i = 0; i < routerTable.size(); i++)
                {
                    UDPsend.clear();
                    UDPsend.str("");
                    formUDPBroadcastData(UDPsend);
                    sendUDPPacket(UDPsend, routerTable[i].destId);
                }
                controlFlag = 1;
                sprintf(logdata, "Control Flag is %d",controlFlag);
                *myLog << FileLogger::e_logType::LOG_INFO << logdata;
            }
            else if(controlFlag == 1)
            {
                TCPrecv.clear();
                receiveTCPPacket(TCPrecv);
                removeDuplicateEntry();
                calcshortpath();
                usleep(10);
                TCPsend.str("DAREADY");
                sendTCPPacket(TCPsend);
                controlFlag = 2;
                sprintf(logdata, "Control Flag is %d",controlFlag);
                *myLog << FileLogger::e_logType::LOG_INFO << logdata;
            }
            else if(controlFlag == 2)
            {
                TCPrecv.clear();
                TCPrecv.str("");
                receiveTCPPacket(TCPrecv);
                if(strcmp(TCPrecv.str().c_str(),"ACKDA") == 0)
                {
                    TCPrecv.str("");
                    TCPrecv.clear();
                    controlFlag = 3;
                    sprintf(logdata, "Control Flag is %d",controlFlag);
                    *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                }
            }
            else if (controlFlag == 3)
            {
                TCPrecv.clear();
                TCPrecv.str("");
                receiveTCPPacket(TCPrecv);
                if(strcmp(TCPrecv.str().c_str(),"QUIT") == 0)
                {
                    sprintf(logdata, "Router %d shutting down. Goodbye!.",routerNumber);
                    *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                    TCPsend.clear();
                    TCPsend.str("QUITACK");
                    sendTCPPacket(TCPsend);
                    //closeRouterPort();

                } else if(strcmp(TCPrecv.str().c_str(),"ACKDA") == 0)
                {
                    TCPrecv.str("");
                    TCPrecv.clear();
                } else
                {
                    UDPsend.clear();
                    UDPsend.str(TCPrecv.str());
                    nextHop = checkPacketData(TCPrecv);
                    if(nextHop!=-1)
                        sendUDPPacket(UDPsend, nextHop);
                }

                //else make packet and send

            }

        }
    }
}

void Router::sendUDPBroadcastData(stringstream &in)
{
    for(int i = 0; i < neighbors.size(); i++)
    {
        if(neighbors[i].neighborPort != receivedUDPPortno)
        {
            sendUDPPacket(in,neighbors[i].neighborID);
        }
    }
}

int Router::checkPacketData(stringstream& in)
{
    stringstream TCPsend;
    char logdata[150];
    int temp1, dst, nextHop = -1;

    if(in.str().length() < 10)
    {
        temp1 = atoi(in.str().substr(0,(in.str().find("\\"))).c_str());

        dst = ntohs(temp1);

        sprintf(logdata,"Packet destined to Router %d.",dst);
        *myLog << FileLogger::e_logType::LOG_INFO << logdata;
        if(dst == routerNumber)
        {
            TCPsend.str("RECEIVED");
            sendTCPPacket(TCPsend);
            nextHop = -1;
        }
        else
        {
            for(int i = 0; i < graph->forwardtable.size(); i++)
            {
                if (dst == graph->forwardtable[i].destId)
                {
                    nextHop = graph->forwardtable[i].nextHop;
                    break;
                }
            }

        }
    }
    else
    {
        *myLog << FileLogger::e_logType::LOG_INFO << "Limited Broadcast Data. UDP Packet dropped.";
        nextHop = -1;
    }


    return nextHop;
}

stringstream& Router::formUDPBroadcastData(stringstream& in)
{
    char data[50];
    memset(data, 0, sizeof(data));
    in << htons(routerNumber) << '|';
    in << htons(routerTable.size()) << '/';
    for(int i = 0; i < routerTable.size(); i++)
    {
        sprintf(data, "%d %d %d %d|", routerTable[i].sourceId, routerTable[i].destId, routerTable[i].cost, htons(routerTable[i].destPort));
        in << data;
    }
    return in;
}

void Router::parseUDPBroadcastData(stringstream& in)
{
    char logdata[100], delimiter;
    string info;
    routingTable r;
    size_t loc;
    int records = 0,temp, i = 0, recvRouter;
    in >> temp;
    recvRouter = ntohs(temp);
    totalRouterDataReceived[recvRouter] = 1;
    sprintf(logdata, "Received from the Router %d",recvRouter);
    *myLog << FileLogger::e_logType::LOG_INFO << logdata;

    in >> delimiter;

    in >> temp;
    records = ntohs(temp);

    sprintf(logdata, "Number of records received - %d",records);
    *myLog << FileLogger::e_logType::LOG_INFO << logdata;

    in >> delimiter;
    while (getline(in, info, '|') && (i < records))
    {
        r.sourceId = atoi(info.substr(0, (info.find(" "))).c_str());
        loc = info.find_first_of(" ");
        info = info.substr(loc + 1);

        r.destId = atoi(info.substr(0, (info.find(" "))).c_str());
        loc = info.find_first_of(" ");
        info = info.substr(loc + 1);

        r.cost = atoi(info.substr(0, (info.find(" "))).c_str());
        loc = info.find_first_of(" ");
        info = info.substr(loc + 1);

        r.destPort = ntohs(atoi(info.c_str()));

        routerTable.push_back(r);

        i++;
    }

}

void Router::removeDuplicateEntry()
{
    int src, dst;
    char logdata[150];
    for(int i = 0; i < routerTable.size(); i++)
    {
        src = routerTable[i].sourceId;
        dst = routerTable[i].destId;
        for(int j = i + 1; j < routerTable.size(); j++)
        {
            if((routerTable[j].sourceId == dst) && (routerTable[j].destId == src))
            {
                routerTable.erase(routerTable.begin() + j);
            }
            if((routerTable[j].sourceId == src) && (routerTable[j].destId == dst))
            {
                routerTable.erase(routerTable.begin() + j);
            }

        }
    }
    *myLog << FileLogger::e_logType::LOG_INFO << "Routing Table after removing Duplicate Entry";
    *myLog << FileLogger::e_logType::LOG_INFO << "SourceId\t\tDestId\t\t  Cost\t\tDestPort";
    for(int i = 0; i < routerTable.size(); i++)
    {
        sprintf(logdata, "\t%d\t\t\t\t%d\t\t\t%d\t\t%d",routerTable[i].sourceId,routerTable[i].destId,routerTable[i].cost,routerTable[i].destPort);
        *myLog << FileLogger::e_logType::LOG_INFO << logdata;
    }
}

void Router::closeRouterPort() {
    close(TCPsocket);
    close(UDPsocket);
}

void Router::sendTCPPacket(stringstream& in)
{
    char logdata[100];
    strcpy(logdata,in.str().c_str());
    *myLog << FileLogger::e_logType::LOG_INFO << "Sending the following data to manager.";
    *myLog << FileLogger::e_logType::LOG_INFO <<logdata;

    send(TCPsocket,in.str().c_str(),sizeofstringstream(in),0);
}

stringstream& Router::receiveTCPPacket(stringstream& in)
{
    char buffer[100];

    memset(buffer, 0, sizeof(buffer));

    *myLog << FileLogger::e_logType::LOG_INFO << "Receiving Node information from the manager.";

    recv(TCPsocket,buffer,sizeof(buffer),0);

    *myLog << FileLogger::e_logType::LOG_INFO <<buffer;

    in << buffer;

    return in;
}

stringstream& Router::receiveUDPPacket(stringstream& in)
{
    char buffer[100], logdata[100];
    memset(buffer, 0, sizeof(buffer));
    sockaddr_in sin;
    socklen_t sin_len = sizeof(sin);
    recvfrom(UDPsocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&sin, &sin_len);
    in.clear();
    in.str(buffer);
    receivedUDPPortno = sin.sin_port;
    sprintf(logdata, "Received %s from neighbor router with port %d", buffer, sin.sin_port);
    *myLog << FileLogger::e_logType::LOG_INFO << logdata;
    return in;
}

void Router::sendUDPPacket(stringstream& in, int destId)
{
    sockaddr_in sout;
    socklen_t sout_len = sizeof(sout);
    char logdata[150];

    bzero((char *) &sout, sizeof(sout));

    sout.sin_family= AF_INET;
    sout.sin_addr.s_addr = htonl(INADDR_ANY);
    sout.sin_port = getdestRouterPort(destId);
    if(sendto(UDPsocket, in.str().c_str(), sizeofstringstream(in), 0, (struct sockaddr *)&sout, sout_len) <0){
        cerr<<("Mismatch in number of bytes received sendToNeighbor");
        _exit(EXIT_FAILURE);
    }
    sprintf(logdata, "Sent %s to router with port %d",in.str().c_str(),getdestRouterPort(destId));
    *myLog << FileLogger::e_logType::LOG_INFO << logdata;
}

uint16_t Router::getdestRouterPort(int destId)
{
    uint16_t  portno = 0;

    for(int i = 0; i < routerTable.size(); i++)
    {
        if(routerTable[i].destId == destId)
            portno = routerTable[i].destPort;
    }
    return portno;
}

bool Router::checklimitedBroadcastStatus()
{
    stringstream send;
    int counter = 0;
    bool status = false;

    for(int i = 0; i < totalRouters; i++)
    {
        if(totalRouterDataReceived[i] == 1)
            counter++;
    }
    if(counter == totalRouters)
    {
        status = true;
        if(LBReadyFlag == 0)
        {
            LBReadyFlag = 1;
            send.str("LBREADY");
            sendTCPPacket(send);
        }

    }

    return status;
}
void Router::calcshortpath()
{
    char logdata[150];
    for(int i = 0; i < routerTable.size(); i++)
    {
        graph->addEdge(routerTable[i].sourceId,routerTable[i].destId,routerTable[i].cost);
    }
    graph->genForwardingTable(routerNumber);
    *myLog << FileLogger::e_logType::LOG_INFO <<"Following is the Shortest Path Forwarding Table.";
    for(int i = 0; i < graph->forwardingTable.size(); i++)
    {
        sprintf(logdata,"%s",graph->forwardingTable[i].c_str());
        *myLog << FileLogger::e_logType::LOG_INFO << logdata;
    }

}

void sig_handler(int signal)
{
    //close all open sockets and any allocated memory.
    stop = 1;
    a.closeRouterPort();
    cout<<"Quiting...\n";
    exit(0);
}

int main(int argc, char *argv[]) {

    a.routerProcess(argv[1]);

}
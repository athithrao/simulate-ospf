////
//// Created by aldrichj on 11/10/16.
////

#include <stdlib.h>
#include "Manager.h"


void sig_handler(int);

volatile sig_atomic_t stop;
int serverTCPSocket = -1;
int clientTCPSocket[MAXROUTERS];
int newSocket = -1;


size_t sizeofstringstream (stringstream &data)
{
    data.seekp(0, ios::end);
    stringstream::pos_type offset = data.tellp();
    return (size_t) offset;
}

bool Manager::areAllRoutersConnected(int control_status)
{
    int count = 0;
    bool status = false;

    if(control_status == 0)
    {
        for(int i = 0; i < totalRouters; i++)
        {
            if(routerUDPPorts[i] !=0)
                count++;
        }
        if(count ==totalRouters)
        {
            status = true;
        }
        else
            status = false;
    }
    else if(control_status == 1)
    {
        for(int i = 0; i < totalRouters; i++)
        {
            if(routerACKStatus[i] !=0)
                count++;
        }
        if(count ==totalRouters)
        {
            status = true;
            for(int i = 0; i < totalRouters; i++) {
                routerACKStatus[i] = 0;
            }
        }
        else
            status = false;
    }

    else if(control_status == 2)
    {
        for(int i = 0; i < totalRouters; i++)
        {
            if(routerLBStatus[i] !=0)
                count++;
        }
        if(count ==totalRouters)
        {
            status = true;
            for(int i = 0; i < totalRouters; i++) {
                routerLBStatus[i] = 0;
            }
        }
        else
            status = false;
    }
    else if(control_status == 3)
    {
        for(int i = 0; i < totalRouters; i++)
        {
            if(routerDAStatus[i] !=0)
                count++;
        }
        if(count ==totalRouters)
        {
            status = true;
        }
        else
            status = false;
    }
    return status;
}


void Manager::readTopologyData(char* fileName)
{
    routerFile.open(fileName, ios::in);
    string line;
    int counter = 0;
    string src,dst,cost;
    size_t loc;
    if (routerFile.is_open()) {

        getline(routerFile, line);
        totalRouters = atoi(line.c_str());

        while(line != "-1")
        {
            counter++;
            getline(routerFile, line);

        }

        topologyDatacount = counter-1;
        routerTopologyData = new string[2*topologyDatacount];
        routerFile.close();
        routerFile.open(fileName, ios::in);
        getline(routerFile, line);
        getline(routerFile, line);
        for(int i = 0; i< topologyDatacount; i++)
        {
            if(line != "-1")
            {
                routerTopologyData[i] = line;
            } else{
                break;
            }
            getline(routerFile, line);
        }
        routerFile.close();
        routerFile.open(fileName, ios::in);
        getline(routerFile, line);
        getline(routerFile, line);

        for(int i = 0; i< topologyDatacount; i++)
        {
            if(line != "-1")
            {
                src = line.substr(0,(line.find(" "))).c_str();
                loc = line.find_first_of(" ");
                line = line.substr(loc + 1);
                dst = line.substr(0,(line.find(" "))).c_str();
                loc = line.find_first_of(" ");
                line = line.substr(loc + 1);
                cost = line.substr(0,(line.find(" "))).c_str();
                line.clear();
                line.append(dst);
                line.append(" ");
                line.append(src);
                line.append(" ");
                line.append(cost);
                routerTopologyData[topologyDatacount + i] = line;
            }
            getline(routerFile, line);
        }
        topologyDatacount *= 2;
        getline(routerFile, line);
        packetInfo p;
        while(line != "-1")
        {
            p.src = atoi(line.substr(0,(line.find(" "))).c_str());
            loc = line.find_first_of(" ");
            line = line.substr(loc + 1);
            p.dest = atoi(line.c_str());
            p.status = false;
            packetData.push_back(p);
            getline(routerFile, line);
        }
    }
    DARAck = 0;
    currPacketData = 0;
    quitAck = 0;
    for(int i = 0; i < MAXROUTERS; i++)
    {
        routerUDPPorts[i] = 0;
        routerACKStatus[i] = 0;
        routerLBStatus[i] = 0;
        routerDAStatus[i] = 0;
    }
    routerFile.close();
}

void Manager::destroyRouters()
{
    for(int i = 0; i < totalRouters; i++)
    {
        kill(children[i], SIGTERM);
    }
}

void Manager::createRouters() {
    pid_t pid;
    int status = 0;
    uint16_t port = 16000;
    char* myargs[3];
    char buffer[33];
    myargs[0] = strdup("router");
    cout<<"Router Forking has started. Please check the <UDPPortNo>.log for process status.\n";
    for(int i = 0;i<totalRouters;i++) {
        memset(buffer, 0x00, sizeof(buffer));
        sprintf(buffer, "%d", port);
        myargs[1] = strdup(buffer);
        myargs[2] = NULL;
        switch ((pid = fork()))
        {
            case -1:
               // fork failed
                cerr<<"fork failed"<<endl;
                break;
            case 0:
               // child code
                children[i] = getpid();
                execv ("router", myargs);
                _exit(EXIT_FAILURE);
            default:
                // parent code
                break;
        }
        port +=1000;
    }

    while ((pid = wait(&status)) > 0)
    {
             cout<<"Exit status of "<< (int)pid <<" was "<<status<<" "<<((status > 0)? "accept" : "reject")<< "\n";
    }

}

stringstream& Manager::formRouterData(stringstream &data, int NodeNo) {

    int temp1;
    string temp;
    data.str("");
    data << htons(NodeNo) << '|'<< htons(totalRouters) << '|';
    for(int i = 0; i < topologyDatacount; i++)
    {
        if((atoi(routerTopologyData[i].substr(0,(routerTopologyData[i].find(" "))).c_str())) == NodeNo)
        {
            data << routerTopologyData[i] << " ";
            temp1 = atoi(routerTopologyData[i].substr((routerTopologyData[i].find(" "))).c_str());
            data << htons(routerUDPPorts[temp1])<< '|';
        }
    }
    return data;
}

stringstream& Manager::formPacketData(stringstream &data, int packetnum)
{
    data.str("");
    data << htons(packetData[packetnum].dest)<<'\\'<<htons(packetData[packetnum].src);
    return data;
}


void Manager::sendRouterInfo(int control_status)
{
    char logdata[150];
    stringstream data;
    int i = 0;
    if(control_status == 0)
    {
        if(areAllRoutersConnected(0))
        {
            *myLog << FileLogger::e_logType::LOG_INFO << "All routers have connected. Sending node address and routing information to routers.";
            for (i = 0; i < totalRouters; i++) {
                //if position is empty
                data.clear();
                data.str("");
                formRouterData(data,i);
                send(clientTCPSocket[i], data.str().c_str(), sizeofstringstream(data), 0);
                sprintf(logdata, "Sent %s to router %d.",data.str().c_str(),i);
                *myLog << FileLogger::e_logType::LOG_INFO << logdata;
            }

        }
    }
    else if (control_status == 1)
    {
        if (areAllRoutersConnected(1))
        {
            *myLog << FileLogger::e_logType::LOG_INFO << "All router READY status received. Sending ACK to start next process.";
            for (i = 0; i < totalRouters; i++) {
                //if position is empty
                usleep(20);
                data.clear();
                data.str("ACKREADY");
                sprintf(logdata, "Sent %s to router %d.",data.str().c_str(),i);
                *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                send(clientTCPSocket[i], data.str().c_str(), sizeofstringstream(data), 0);
            }

        }
        if (areAllRoutersConnected(2))
        {
            *myLog << FileLogger::e_logType::LOG_INFO << "All router Limited Broadcast Completed. Sending ACK to start next process.";
            for (i = 0; i < totalRouters; i++) {
                //if position is empty
                usleep(20);
                data.clear();
                data.str("ACKLB");
                sprintf(logdata, "Sent %s to router %d.",data.str().c_str(),i);
                *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                send(clientTCPSocket[i], data.str().c_str(), sizeofstringstream(data), 0);
            }

        }
        if (areAllRoutersConnected(3))
        {
            if(DARAck  == 0)
            {
                *myLog << FileLogger::e_logType::LOG_INFO << "All router shortest path tree algorithm execution completed. Contacting router for packet exchange.";
                DARAck = 1;
                for (i = 0; i < totalRouters; i++) {
                    //Here we need to send data.
                    usleep(20);
                    data.clear();
                    data.str("ACKDA");
                    sprintf(logdata, "Sent %s to router %d.",data.str().c_str(),i);
                    *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                    send(clientTCPSocket[i], data.str().c_str(), sizeofstringstream(data), 0);
                }

            }
            if(currPacketData < packetData.size())
            {
                usleep(50000);
                data.clear();
                data.str("");
                formPacketData(data, currPacketData);
                sprintf(logdata, "Sent %s to router %d.",data.str().c_str(),packetData[currPacketData].src);
                *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                sprintf(logdata, "Current Packet Data number %d.",currPacketData);
                *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                send(clientTCPSocket[packetData[currPacketData].src], data.str().c_str(), sizeofstringstream(data), 0);
            }
            else
            {
                if(DARAck  == 1)
                {
                    DARAck = 2;
                    usleep(50000);
                    *myLog << FileLogger::e_logType::LOG_INFO << "All destination routers have received the packets";
                    *myLog << FileLogger::e_logType::LOG_INFO << "Sending QUIT to start exit sequence.";
                    for (i = 0; i < totalRouters; i++) {
                        //Here we need to send data.
                        usleep(20);
                        data.clear();
                        data.str("QUIT");
                        sprintf(logdata, "Sent %s to router %d.",data.str().c_str(),i);
                        *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                        send(clientTCPSocket[i], data.str().c_str(), sizeofstringstream(data), 0);
                    }
                }
            }
        }
    }
}

void Manager::serverTCP() {
    int option = 1;
    int status;
    int max_sd, sd;
    int i = 0;
    fd_set readfds;
    char temp;
    uint16_t temp1;
    struct sockaddr_in sin, clientSocketAddress;
    int defaultPort = 15000;
    char buffer[140],logdata[150];
    socklen_t socketAddLen;

    myLog = new FileLogger(1,"1.0.0", "Manager.out");

    stringstream data;

    cout<<"Manager Process Started. Please check Manager.out for process status.\n";

    for (i = 0; i < totalRouters; i++) {
        clientTCPSocket[i] = 0;
        routerUDPPorts[i] = 0;
    }

    serverTCPSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverTCPSocket < 0) {
        std::cout << "Error: Socket Establishment. Quitting Program.. \n";
        exit(1);
    }

    if (setsockopt(serverTCPSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &option, sizeof(option)) < 0) {
        std::cout << "Error: Socket to allocate multiple connections. Quitting Program.. \n";
        exit(1);
    }

    bzero((char *) &sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sin.sin_port = htons(defaultPort);

    socklen_t size = sizeof(sin);

    status = ::bind(serverTCPSocket, (struct sockaddr *) &sin, size);

    if (serverTCPSocket < 0) {
        std::cout << "Error Binding. Quitting Program.. \n";
        close(serverTCPSocket);
        exit(1);
    }

    *myLog << FileLogger::e_logType::LOG_INFO << "Manager TCP Server Started.";
    sprintf(logdata, "Manager IP Address - %s    Port No - %d   Total Routers - %d", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port), totalRouters);
    *myLog << FileLogger::e_logType::LOG_INFO <<logdata;
    *myLog << FileLogger::e_logType::LOG_INFO << "Listening to routers for TCP connections.";


    listen(serverTCPSocket, MAXROUTERS);

    socketAddLen = sizeof(clientSocketAddress);

    while (!stop) {
        signal(SIGINT, sig_handler);
        FD_ZERO(&readfds);

        FD_SET(serverTCPSocket, &readfds);
        max_sd = serverTCPSocket;

        for (i = 0; i < totalRouters; i++) {
            sd = clientTCPSocket[i];

            if (sd > 0)
                FD_SET(sd, &readfds);

            if (sd > max_sd)
                max_sd = sd;
        }

        status = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((status < 0) && (errno != EINTR)) {
            cout << "Select function error. Quitting Program..\n";
            exit(1);
        }

        if (FD_ISSET(serverTCPSocket, &readfds)) {

            if ((newSocket = accept(serverTCPSocket, (struct sockaddr *) &clientSocketAddress, &socketAddLen)) < 0) {
                cout << "New Socket acceptance error. Quitting Program..\n";
                exit(1);
            }

            memset(buffer, 0, sizeof(buffer));
            //send new connection greeting message
            recv(newSocket, buffer, sizeof(buffer), 0);
            data.clear();
            //get the UDP Port num from the router.
            data << buffer;

            data >> temp;

            data >> temp1;

            for (i = 0; i < totalRouters; i++) {
                //if position is empty
                if (clientTCPSocket[i] == 0) {
                    clientTCPSocket[i] = newSocket;
                    routerUDPPorts[i] = ntohs(temp1);
                    break;
                }
            }

            sendRouterInfo(0);

        }

        for (i = 0; i < MAXROUTERS; i++) {
            sd = clientTCPSocket[i];
            if (FD_ISSET(sd, &readfds)) {
                //Check if it was for closing , and also read the incoming message
                memset(buffer, 0, sizeof(buffer));

                recv(sd, buffer, sizeof(buffer), 0);
                if(strcmp(buffer,"READY") == 0)
                {
                    sprintf(logdata, "READY status received from Router %d.",i);
                    *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                    routerACKStatus[i] = 1;
                }
                if(strcmp(buffer,"LBREADY") == 0)
                {
                    sprintf(logdata, "Limited Broadcast complete status received from Router %d.",i);
                    *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                    routerLBStatus[i] = 1;
                }
                if(strcmp(buffer,"DAREADY") == 0)
                {
                    sprintf(logdata, "Dijkstra’s shortest path algorithm complete status received from Router %d.",i);
                    *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                    routerDAStatus[i] = 1;
                }
                if(strcmp(buffer,"RECEIVED") == 0)
                {
                    sprintf(logdata, "Destination Router %d has received the packet.",packetData[currPacketData].dest);
                    *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                    currPacketData++;
                }
                if(strcmp(buffer,"QUITACK") == 0)
                {
                    sprintf(logdata, "QUITACK received from Router %d.",i);
                    *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                    quitAck++;
                    if(quitAck == totalRouters)
                    {
                        sprintf(logdata, "Quitting Routers and Manager.");
                        *myLog << FileLogger::e_logType::LOG_INFO << logdata;
                        destroyRouters();
                        exit(0);
                    }
                }
                sendRouterInfo(1);
            }
        }
    }
}


void sig_handler(int signal)
{
    //close all open sockets and any allocated memory.
    stop = 1;
    close(serverTCPSocket);
    close(newSocket);
    for(int i = 0; i< MAXROUTERS;i++)
    {
        close(clientTCPSocket[i]);
    }
    cout<<"Quiting...\n";
    exit(0);
}

int main(int argc, char *argv[]) {

    // verify input
    if(argc != 2){
        cout<<"Invalid number of arguments"<<endl;
        cout<<"Usage: $manager <input file>"<<endl;
        return -1;
    }

    Manager manager;
    stringstream data;
    manager.readTopologyData(argv[1]);


    thread routerManager = manager.createRoutersThread();
    thread TCPManager = manager.serverTCPThread();


    TCPManager.join();
    routerManager.join();


    return 0;
}
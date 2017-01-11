Project Description

In this project you will use unix processes to simulate routers in a network running link state routing using the Dijkstra algorithm. You will then test your implementation by sending packets between nodes in the network and printing a trace of the path each packet follows. For this project, all processes will run on the same machine.

Your project contains two types of processes. First is the manager process. Although not part of the network, this process is responsible for creating the network after reading a configuration file with the description of the network to be created. The description includes the number of routers in the simulated network, the links between routers and their cost.

The second type of process is the router process. These implement the network and are created by the manager by forking one unix process per router. The manager communicates with the routers via TCP.  Routers emulate network links via UDP associations. In other words, routers communicate with their neighbors via UDP. The figure below show an example network.
sample network
The Manager Process
The network is managed by a process called the manager. The manager takes as argument a single file. You will run the manager as follows.

$manager <input file>
The input file contains a network topology description. The format is as follows:

The first line of this table contains a single integer N. This means that there are N nodes in the network, whose addresses are 0, 1, 2, ..., N-1.
This line is followed by several lines, one for each point-to-point link in the network. Each line has 3 integers separated by whitespace <X, Y, C>. X and Y are node numbers between 0 and N-1, and C is a positive integer that represents the cost of a link between X and Y.
A -1, which indicates the end of the list of links in the input file
Here is a sample network topology description: 
 
10 
0 9 40 
0 4 60 
0 2 40 
1 8 70 
2 6 70 
3 9 20 
3 8 70 
3 5 70 
4 7 70 
4 6 40 
5 9 70 
5 7 40 
7 9 60 
8 9 70
-1
After the manager reads the file, the following things will happen:

The manager will spawn one Unix process for each router in the network.
Each router will create a datagram (UDP) socket. This socket will be used to connect to neighboring nodes, implement the routing protocol and transmit data. Before any protocol message exchange can happen, the router must be told several pieces of information:
It's own address (a number between 0..N-1)
Its connectivity table (who its neighbors are, what are the link costs to each neighbor and the UDP port number for each neighbor).
You have to implement a simple protocol between the manager and each router so that the manager can tell the router this information. You should design the message formats for exchanging this information. One way of doing this is:
After each router starts up, it opens a TCP connection to the manager.
The router sends a message to the manager telling it what its own UDP port is, a request to the manager to be assigned a node address and to be given the appropriate connectivity table.
At this point each router starts exchanging routing messages to establish routes to all other routers, as described below.
After routes are established, the manager continues to read the input file for instructions on sending packets from a source to a destination. The commands in the file will look like this (note that this will come right after the connectivity table):
0 5
3 9
...
-1

The first number indicates the source of a packet and the second number the destination. Once the manager reads a src-dst pair, it will instruct the appropriate source router to send a packet to the destination router. It is a good idea for the manager to sleep for a while (say 1-5s) before reading the next entry in the file. The manager will continue to read and execute instructions from the input file until the value -1 is reached. At that point it will sleep for a while to let all packet transmissions finish (you need to figure out how long) and then wake up and send a "Quit" message to all the routers. At this point all the processes should exit, children first and finally the manager.
Manager Process Output

The manager outputs messages at each step of its operation. The messages are appended into a file named manager.out, to separate them from the messages output by the other processes. It is up to you to decide what messages the manager will print, but we suggest you print messages at every step because it will greatly help you in debugging. At the very least we expect to see a message every-time the manager sends or receives a message from another process. We recommend that you print messages one per line and add white space and blank lines appropriately to make the file easily readable.
Router Process
As described above, the router process, once it gets created, does the following:
Create a UDP port
Create a TCP port and connect to the manager process
Send the manager the port number of the UDP port and a request for a node address and the associated connectivity table
Receive the above information and send a "Ready!" message to the manager, indicating that it is ready to accept link establishment requests from neighbor nodes
Wait for a message from the manager indicating it is safe to try to reach its neighbors
Send a link request to each of its neighbors and wait for an ACK
Once ACKs from all neighbors are received, send a message to the manager indicating that link establishment is complete
Wait for the manager to indicate that all links from all routers have been established and the network is up
Send its own link state packet (LSP) to all its neighbors.
Perform limited broadcast on all LSPs received from other routers (limited broadcast means send the packet to all interfaces except the one it was received on), while filtering out duplicates (note that the same LSP may be received many times)
Once we have seen LSPs from all other routers, calculate the shortest path tree (SPT) using the Dijkstra algorithm
Output the SPT table by appending it in the file named <routerID>.out
Update the forwarding table
At this point the router process is ready to accept and forward data packets. The router process sends a message to the manager and waits for instructions on what data packets to send. The manager, after reading the input file, will send instructions to the router to originate a data packet. The packet will contain the destination router address. Once a router receives instructions to originate a packet, the router will look up its forwarding table and send the packet to the appropriate link. Note that data packets may be initiated by the manager or may arrive on one of the network links. The router should be ready to process both.

When a router receives a data packet, the router looks at the destination address in the packet. If the address is the router's own address, then the router appends a message to the output file and removes the packet from the network. If the packet is destined for a different router, then the router does a lookup and forwards the packet to the appropriate link.
The router process exits when it receives an "Quit" message from the manager.
Router Process Output

Similar to the manager, the router outputs detailed messages about what happens. Routers write their output in separate files (one for each router) called routerN.out, where N is the node ID of the router. Again, at the very least we expect to see a message everytime the manager sends or receives a message from another process and most definitely the routing table and information about data packets being forwarded.
The Routing Algorithm

You will implement the link state routing algorithm described in the book. The book gives high-level pseudo code, which you will have to translate into a C/C++ program. It is extremely important that you understand this algorithm well, otherwise you will not be able to correctly implement it. You are free to use any data structures you like to implement the algorithm. An efficient implementation is a plus, but don't spend too much time optimizing your code. At this point correctness is more important than efficiency.

Before running the algorithm, you will need to wait until you receive LSPs from every other router. In this project we will allow the manager to tell each router how many other nodes are there in the network (an alternative would be for each router to wait some time until no more LSPs are received). Each router will then calculate the shortest path tree to every other router. Then, you will use that information to fill out the forwarding table. Once you have a complete forwarding table, every router will print it in the router's output file.
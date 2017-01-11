# CS457Project3
Project 3

CS457 - Project 3

Group Members - Joshua Fuller, Athith Amarnath & Jeremy Aldrich

1. To Build
make

2. To Run
./manager <nodes.txt>

nodes.txt represents the topology of the network and the start/end routers for a packet being forwarded.

Example format:

4
0 1 40
0 3 60
1 2 70
2 3 40
3 1 60
-1
0 2
1 3
-1

Assumptions -
1. The nodes.txt file needs to be formatted correctly. Please make sure there are no spaces at the end of each line. Examples nodes.txt are attached with the project for reference.
2. If the total number of routers for the test is N, the router number (node address) assigned to each router is 0 to N-1 once connected to the manager. 
2. No nodes should have edges pointing towards themselves. 
4. Manager process creates a Manager.out File and records communication between the manager and the router.
3. The UDP Port numbers are assigned by the manager process and starts from 16000. Every router is given an increment of 1000; i.e. routers with UDP Port number 16000, 17000, 18000 and so on.
4. If the binding fails at the router with the given UDP port number, the router process will increment the UDP port number by 1 until the binding is successful. Each router process then creates <UDP Port no>.out file to record the output.
5. There are limited broadcast (flooding) packets on the network even after completing the shortest path calculation. Those packets are dropped.
6. Estimated time to complete the process would be less than 1 second for 10 routers. :). 

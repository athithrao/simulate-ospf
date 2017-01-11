//
// Created by jeremy on 11/27/16.
//

#ifndef CS457PROJECT3_GRAPH_H
#define CS457PROJECT3_GRAPH_H

#include <iostream>
#include <vector>

using namespace std;
class Graph {

    struct lsp {
        int destId;
        int cost;
        int nextHop;
    };

public:

    Graph(int num){
        numNodes = num;
        adjMatrix.resize(num);
        parent.resize(num);


        for (int k = 0; k <num ; ++k) {
            adjMatrix[k].resize(num);

        }


        for (int i = 0; i < numNodes ; ++i) {
            for (int j = 0; j < numNodes ; ++j) {

                adjMatrix[i][j] = 0;

            }

        }

    };
    vector<string> forwardingTable;

    vector<lsp> forwardtable;

    //vector<int> dijk(int A, int B, vector< vector<int> > adj);

    void addEdge(int X, int Y, int cost);
    void printAdjMatrix();
    vector<vector<int>> getAdjMatrix(){
        return adjMatrix;
    }
    void getPath(vector<int>& k, int src, int dst);


    void genForwardingTable(int src);


private:

    vector<int> parent;



    void getPathHelper(vector<int>& k,int j);
    void dijkstra(int src);



    //const int inf = 0;
    int numNodes;
    std::vector<std::vector<int>> adjMatrix;
    int minDistance(int dist[], bool sptSet[]);

    int printSolution(int dist[], int n, int parent[]);


};


#endif //CS457PROJECT3_GRAPH_H

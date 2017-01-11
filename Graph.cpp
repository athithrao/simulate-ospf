/**
 * Created: http://www.geeksforgeeks.org/printing-paths-dijkstras-shortest-path-algorithm/
 *  Modified by Jeremy Aldrich
 *
 * Manages the graph including forming an adj list to use in Dijkstra's algorithm
 *
 *
 *
 */

// A C / C++ program for Dijkstra's single source shortest
// path algorithm. The program is for adjacency matrix
// representation of the graph.
#include <stdio.h>
#include <limits.h>

// Number of vertices in the graph
#define V 9

// A utility function to find the vertex with minimum distance
// value, from the set of vertices not yet included in shortest
// path tree

//using namespace std;
#include "Manager.h"
#include "Graph.h"
#include <stdlib.h>
#include <math.h>
#include <algorithm>


/**
 * Adds an edge to the adj matrix
 * Since we're dealing with undirected graphs we add an edge in both directions
 * @param X
 * @param Y
 * @param cost
 */

void Graph::addEdge(int X, int Y, int cost){





    if (adjMatrix[X][Y] == 0) {
        adjMatrix[X][Y] = cost;
        adjMatrix[Y][X] = cost;

    }


}



int Graph::minDistance(int dist[], bool sptSet[])
{
    // Initialize min value
    int min = INT_MAX, min_index;

    for (int v = 0; v < numNodes; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
}



// Function to print shortest path from source to j
// using parent array
void Graph::getPathHelper(vector<int>& k,int j)
{
    // Base Case : If j is source
    if (parent[j]==-1)
        return;

    getPathHelper(k,parent[j]);

    //printf("%d ", j);
    k.push_back(j);
}

// A utility function to print the constructed distance
// array
int Graph::printSolution(int dist[], int n, int parent[])
{
    int src = 0;
    printf("Vertex\t  Distance\tPath");
    for (int i = 1; i < numNodes; i++)
    {

        printf("\n%d -> %d \t\t %d\t\t%d ", src, i, dist[i], src);
        //printPath(i);
    }
    return 0;
}

// Funtion that implements Dijkstra's single source shortest path
// algorithm for a graph represented using adjacency matrix
// representation
void Graph::dijkstra(int src)
{
    int dist[numNodes];  // The output array. dist[i] will hold
                  // the shortest distance from src to i

    // sptSet[i] will true if vertex i is included / in shortest
    // path tree or shortest distance from src to i is finalized
    bool sptSet[numNodes];

    // Parent array to store shortest path tree
    //int parent[numNodes];

    // Initialize all distances as INFINITE and stpSet[] as false
    for (int i = 0; i < numNodes; i++)
    {
        parent[src] = -1;
        dist[i] = INT_MAX;
        sptSet[i] = false;
    }

    // Distance of source vertex from itself is always 0
    dist[src] = 0;

    // Find shortest path for all vertices
    for (int count = 0; count < numNodes-1; count++)
    {
        // Pick the minimum distance vertex from the set of
        // vertices not yet processed. u is always equal to src
        // in first iteration.
        int u = minDistance(dist, sptSet);

        // Mark the picked vertex as processed
        sptSet[u] = true;

        // Update dist value of the adjacent vertices of the
        // picked vertex.
        for (int v = 0; v < numNodes; v++)

            // Update dist[v] only if is not in sptSet, there is
            // an edge from u to v, and total weight of path from
            // src to v through u is smaller than current value of
            // dist[v]
            if (!sptSet[v] && adjMatrix[u][v] &&
                dist[u] + adjMatrix[u][v] < dist[v])
            {
                parent[v]  = u;
                dist[v] = dist[u] + adjMatrix[u][v];
            }
    }

    // print the constructed distance array
    //printSolution(dist, numNodes, parent);
}


/**
 * Simple print of the adj matrix
 */
void Graph::printAdjMatrix() {
    cout<<"  ";
    for (int k = 0; k <numNodes; ++k) {
        cout<<k<<"  ";
    }
    for (int i = 0; i < numNodes; ++i) {
        cout<<endl;
        cout<<i<<" ";
        for (int j = 0; j < numNodes; ++j) {
            cout<<adjMatrix[i][j]<<" ";

        }

    }
    cout<<endl;

}



void Graph::getPath(vector<int> &k, int src, int dst) {

        parent.clear();
        dijkstra(src);
        getPathHelper(k,dst);
        k.insert(k.begin(),src);

}
/**
 * Generates a forwarding table for the src node.
 * Pre condition: that the adj matrix is filled out.
 * @param src (forwarding table generated for this node)
 */
void Graph::genForwardingTable(int src) {
    // path
    vector<int>k;
    lsp p;

    int totalNodes = adjMatrix[0].size();



    //(Destination, Cost, NextHop)
    // Going from 2 to 0
    // (0,0,-),()


    for (int i = 0; i < totalNodes ; ++i) {
        k.clear();
        // (dst,cost,nexthop)
        string table;
        getPath(k,src,i);
        if(src == i)
        {
            p.destId = src;
            p.cost = 0;
            p.nextHop = -1;
            table = "( " + std::to_string(src) + " , 0 , - )";
        }
        else
        {
            p.destId = i;
            p.cost = adjMatrix[src][k[1]];
            p.nextHop = k[1];
            table = "( "+std::to_string(i)+" , "+std::to_string(adjMatrix[src][k[1]])+" , "+std::to_string(k[1])+" )";
        }

        forwardtable.push_back(p);
        forwardingTable.push_back(table);
    }
}
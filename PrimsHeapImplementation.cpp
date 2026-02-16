/*
 * PRIM'S MINIMUM SPANNING TREE ALGORITHM
 * For CS 481/581 Priority Queue Project
 *
 * Uses PairingHeap implementation EXACTLY as provided by teammate.
 * NOTE: PairingHeap::extract_min() returns ONLY the key.
 *       This is a known interface issue and should be fixed by heap lead.
 *       Workaround used here for integration/testing purposes.
 */

#include <bits/stdc++.h>
#include "binomial_heap.hpp"
#include "pairingHeap.hpp"

using namespace std;

/* =======================
   GRAPH IMPLEMENTATION
   ======================= */

struct Graph {
    int V;
    vector<vector<pair<int,int>>> adj;

    Graph(int vertices) : V(vertices), adj(vertices) {}

    void addEdge(int u, int v, int w) {
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }

    const vector<pair<int,int>>& neighbors(int u) const {
        return adj[u];
    }
};

/* =======================
   PRIM'S ALGORITHM
   ======================= */

void primMST_Pairing(const Graph& graph, int start, PairingHeap& pq) {
    int V = graph.V;

    vector<int> key(V, INT_MAX);
    vector<int> parent(V, -1);
    vector<bool> inHeap(V, true);
    

    //Choose heap type:

    //Uncomment this to use for PAIRING HEAP
    vector<HeapNode*> heap_nodes(V);

    //Uncoomment this to use for BINOMIAL HEAP
    //vector<Binomial_Heap_Node*> heap_nodes(V);


    key[start] = 0;

    for (int v = 0; v < V; v++) {
        heap_nodes[v] = pq.insert(key[v], v);
    }

    while (!pq.empty()) {
        HeapNode* min_node = pq.extract_min();
        int u = min_node->value;
        int min_key = min_node->key;
        delete min_node;

        /*
        for (int i = 0; i < V; i++) {
            if (inHeap[i] && key[i] == min_key) {
                u = i;
                break;
            }
        }
        */

        
        //if (u == -1) continue;
        inHeap[u] = false;
        

        for (auto [v, weight] : graph.neighbors(u)) {
            if (inHeap[v] && weight < key[v]) {
                key[v] = weight;
                parent[v] = u;
                pq.decrease_key(heap_nodes[v], weight);
            }
        }
    }

    /*
    cout << "Edges in MST:\n";
    int total = 0;
    for (int v = 0; v < V; v++) {
        if (parent[v] != -1) {
            cout << parent[v] << " - " << v
                 << " (weight " << key[v] << ")\n";
            total += key[v];
        }
    }
    cout << "Total weight: " << total << endl;
    */
}

void primMST_Binomial(const Graph& graph, int start, Binomial_Heap& pq) {
    int V = graph.V;

    vector<int> key(V, INT_MAX);
    vector<int> parent(V, -1);
    vector<bool> inHeap(V, true);
    

    //Choose heap type:

    //Uncomment this to use for PAIRING HEAP
    //vector<HeapNode*> heap_nodes(V);

    //Uncoomment this to use for BINOMIAL HEAP
    vector<Binomial_Heap_Node*> heap_nodes(V);


    key[start] = 0;

    for (int v = 0; v < V; v++) {
        heap_nodes[v] = pq.insert(key[v], v);
    }

    while (!pq.empty()) {
        int min_key = pq.extract_min();

        // Workaround: find which vertex has this key and is still in heap
        int u = -1;
        for (int i = 0; i < V; i++) {
            if (inHeap[i] && key[i] == min_key) {
                u = i;
                break;
            }
        }

        if (u == -1) continue;
        inHeap[u] = false;

        for (auto [v, weight] : graph.neighbors(u)) {
            if (inHeap[v] && weight < key[v]) {
                key[v] = weight;
                parent[v] = u;
                pq.decrease_key(heap_nodes[v], weight);
            }
        }
    }

    
    //cout << "Edges in MST:\n";
    int total = 0;
    for (int v = 0; v < V; v++) {
        if (parent[v] != -1) {
            //cout << parent[v] << " - " << v
                 //<< " (weight " << key[v] << ")\n";
            total += key[v];
        }
    }
        
    cout << "Total weight: " << total << endl;
    
}

Graph generateGraph(int V, int E) {
    Graph g(V);
    srand(0); 
    cout << "Number of vertices: " << V << endl;
    cout << "Number of edges: " << E << endl;
    for (int i = 0; i < E; i++) {
        int u = rand() % V; //random vertex between 0 and V-1
        int v = rand() % V; //random veertex between 0 and V-1
        int w = rand() % 100 + 1; //random edge weight between 1-100

        if (u != v) {
            g.addEdge(u, v, w);
        }
    }
    return g;
}

int main() {
    /*
    Graph g(5);
    g.addEdge(0, 1, 2);
    g.addEdge(0, 3, 6);
    g.addEdge(1, 2, 3);
    g.addEdge(1, 3, 8);
    g.addEdge(1, 4, 5);
    g.addEdge(2, 4, 7);
    g.addEdge(3, 4, 9);
    */

    
    int V = 10000;
    int E = 50000;
    Graph g = generateGraph(V, E);
    //uncomment the necessary comments to test pairing
    //PairingHeap pairing_pq; 

    //uncomment the necessary comments to test binomial
    Binomial_Heap binomial_pq; 
    
    auto start = chrono::high_resolution_clock::now();

    //primMST_Pairing(g, 0, pairing_pq);      //uncomment for pairing 
    primMST_Binomial(g, 0, binomial_pq);  //uncomment for binomial 
    //pairing_pq.print_stats();               //uncomment for pairing 
    binomial_pq.print_stats();            //uncomment for binomial 
    auto end = chrono::high_resolution_clock::now();
    cout << "Total time: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " μs\n";
    return 0;
}

#include <bits/stdc++.h>
#include "binomial_heap.hpp"
using namespace std;

/* =======================
   PAIRING HEAP
   ======================= */

struct HeapNode {
    int key, value;
    HeapNode *parent, *child, *sibling;

    HeapNode(int k, int v)
        : key(k), value(v), parent(nullptr), child(nullptr), sibling(nullptr) {}

    void addChild(HeapNode *node) {
        node->parent = this;
        node->sibling = child;
        child = node;
    }
};

struct PairingHeap {
    HeapNode *root;
    PairingHeap() : root(nullptr) {}

    HeapNode* insert(int key, int value) {
        HeapNode* node = new HeapNode(key, value);
        root = merge(root, node);
        return node;
    }

    int extract_min() {
        if (!root) throw runtime_error("Empty heap");
        int val = root->value;
        HeapNode* old = root;
        root = merge_pairs(root->child);
        if (root) root->parent = nullptr;
        delete old;
        return val;
    }

    void decrease_key(HeapNode* node, int new_key) {
        if (!node || new_key > node->key) return;
        node->key = new_key;
        if (node == root) return;
        cut(node);
        root = merge(root, node);
    }

    bool empty() const { return root == nullptr; }

private:
    HeapNode* merge(HeapNode* a, HeapNode* b) {
        if (!a) return b;
        if (!b) return a;
        if (a->key <= b->key) {
            a->addChild(b);
            return a;
        } else {
            b->addChild(a);
            return b;
        }
    }

    HeapNode* merge_pairs(HeapNode* first) {
        if (!first) return nullptr;
        if (!first->sibling) {
            first->parent = nullptr;
            return first;
        }

        vector<HeapNode*> trees;
        HeapNode* curr = first;

        while (curr) {
            HeapNode* a = curr;
            HeapNode* b = curr->sibling;

            if (b) {
                HeapNode* next = b->sibling;
                a->sibling = b->sibling = nullptr;
                a->parent = b->parent = nullptr;
                trees.push_back(merge(a, b));
                curr = next;
            } else {
                a->sibling = nullptr;
                a->parent = nullptr;
                trees.push_back(a);
                break;
            }
        }

        HeapNode* result = trees.back();
        for (int i = trees.size() - 2; i >= 0; --i)
            result = merge(trees[i], result);

        return result;
    }

    void cut(HeapNode* node) {
        if (!node->parent) return;
        HeapNode* p = node->parent;
        if (p->child == node) {
            p->child = node->sibling;
        } else {
            HeapNode* prev = p->child;
            while (prev && prev->sibling != node)
                prev = prev->sibling;
            if (prev) prev->sibling = node->sibling;
        }
        node->parent = node->sibling = nullptr;
    }
};

/* =======================
   GRAPH
   ======================= */

struct Graph {
    int V;
    vector<vector<pair<int,int>>> adj;

    Graph(int v) : V(v), adj(v) {}

    void addEdge(int u, int v, int w) {
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }
};

/* =======================
   STATS
   ======================= */

struct Stats {
    long long insert_time = 0;
    long long extract_time = 0;
    long long decrease_time = 0;

    long long insert_count = 0;
    long long extract_count = 0;
    long long decrease_count = 0;

    long long nodes_allocated = 0;
};

/* =======================
   DIJKSTRA - PAIRING
   ======================= */

void dijkstra_pairing(const Graph& g, int src, Stats& stats) {

    const int INF = INT_MAX;
    int V = g.V;

    vector<int> dist(V, INF);
    vector<bool> done(V, false);
    vector<HeapNode*> nodes(V);

    PairingHeap pq;
    dist[src] = 0;

    for (int i = 0; i < V; i++) {
        auto t1 = chrono::high_resolution_clock::now();
        nodes[i] = pq.insert(dist[i], i);
        auto t2 = chrono::high_resolution_clock::now();
        stats.insert_time += chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
        stats.insert_count++;
        stats.nodes_allocated++;
    }

    while (!pq.empty()) {

        auto t1 = chrono::high_resolution_clock::now();
        int u = pq.extract_min();  // returns vertex id
        auto t2 = chrono::high_resolution_clock::now();

        stats.extract_time += chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
        stats.extract_count++;

        if (done[u]) continue;
        done[u] = true;

        for (auto [v, w] : g.adj[u]) {
            if (!done[v] && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;

                auto t3 = chrono::high_resolution_clock::now();
                pq.decrease_key(nodes[v], dist[v]);
                auto t4 = chrono::high_resolution_clock::now();

                stats.decrease_time += chrono::duration_cast<chrono::microseconds>(t4 - t3).count();
                stats.decrease_count++;
            }
        }
    }
}

/* =======================
   DIJKSTRA - BINOMIAL (SAFE VERSION)
   ======================= */

void dijkstra_binomial(const Graph& g, int src, Stats& stats) {

    const int INF = INT_MAX;
    int V = g.V;

    vector<int> dist(V, INF);
    vector<bool> done(V, false);
    vector<Binomial_Heap_Node*> nodes(V);

    Binomial_Heap pq;
    dist[src] = 0;

    for (int i = 0; i < V; i++) {
        auto t1 = chrono::high_resolution_clock::now();
        nodes[i] = pq.insert(dist[i], i);
        auto t2 = chrono::high_resolution_clock::now();
        stats.insert_time += chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
        stats.insert_count++;
        stats.nodes_allocated++;
    }

    while (!pq.empty()) {

        auto t1 = chrono::high_resolution_clock::now();
        int extracted = pq.extract_min();  // may return KEY not VALUE
        auto t2 = chrono::high_resolution_clock::now();

        stats.extract_time += chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
        stats.extract_count++;

        // SAFE mapping: find vertex whose dist matches extracted key
        int u = -1;
        for (int i = 0; i < V; i++) {
            if (!done[i] && dist[i] == extracted) {
                u = i;
                break;
            }
        }

        if (u == -1) continue;

        done[u] = true;

        for (auto [v, w] : g.adj[u]) {
            if (!done[v] && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;

                auto t3 = chrono::high_resolution_clock::now();
                pq.decrease_key(nodes[v], dist[v]);
                auto t4 = chrono::high_resolution_clock::now();

                stats.decrease_time += chrono::duration_cast<chrono::microseconds>(t4 - t3).count();
                stats.decrease_count++;
            }
        }
    }
}

/* =======================
   RANDOM GRAPH
   ======================= */

Graph generateGraph(int V, int E) {
    Graph g(V);
    srand(0);
    for (int i = 0; i < E; i++) {
        int u = rand() % V;
        int v = rand() % V;
        int w = rand() % 100 + 1;
        if (u != v) g.addEdge(u, v, w);
    }
    return g;
}

/* =======================
   MAIN
   ======================= */

int main() {

    int V = 10000;
    int E = 50000;

    Graph g = generateGraph(V, E);

    // Pairing
    cout << "===== DIJKSTRA: Pairing Heap =====\n";
    Stats ps;
    auto s1 = chrono::high_resolution_clock::now();
    dijkstra_pairing(g, 0, ps);
    auto e1 = chrono::high_resolution_clock::now();

    cout << "Total runtime: "
         << chrono::duration_cast<chrono::milliseconds>(e1 - s1).count()
         << " ms\n";
    cout << "Insert: " << ps.insert_count << " ops | " << ps.insert_time << " us\n";
    cout << "Extract: " << ps.extract_count << " ops | " << ps.extract_time << " us\n";
    cout << "Decrease: " << ps.decrease_count << " ops | " << ps.decrease_time << " us\n";
    cout << "Estimated memory: "
         << (ps.nodes_allocated * sizeof(HeapNode)) / 1024.0 << " KB\n\n";

    // Binomial
    cout << "===== DIJKSTRA: Binomial Heap =====\n";
    Stats bs;
    auto s2 = chrono::high_resolution_clock::now();
    dijkstra_binomial(g, 0, bs);
    auto e2 = chrono::high_resolution_clock::now();

    cout << "Total runtime: "
         << chrono::duration_cast<chrono::milliseconds>(e2 - s2).count()
         << " ms\n";
    cout << "Insert: " << bs.insert_count << " ops | " << bs.insert_time << " us\n";
    cout << "Extract: " << bs.extract_count << " ops | " << bs.extract_time << " us\n";
    cout << "Decrease: " << bs.decrease_count << " ops | " << bs.decrease_time << " us\n";
    cout << "Estimated memory: "
         << (bs.nodes_allocated * sizeof(Binomial_Heap_Node)) / 1024.0 << " KB\n";

    return 0;
}

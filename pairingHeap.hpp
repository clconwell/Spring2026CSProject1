/*
 * PAIRING HEAP IMPLEMENTATION
 * CS 481/581
 * 
 * PUBLIC INTERFACE (use these in Dijkstra/Prim):
 * ------------------------------------------------
 * PairingHeap pq;                              // Create heap
 * HeapNode* node = pq.insert(key, value);      // Insert (key=distance/weight, value=vertex_id)
 *                                              // IMPORTANT: Store returned node pointer!
 * int min_key = pq.find_min();                 // Peek at minimum key
 * int min_key = pq.extract_min();              // Remove and return minimum key
 * pq.decrease_key(node, new_key);              // Decrease key of a node
 * bool is_empty = pq.empty();                  // Check if heap is empty
 * pq.print_stats();                            // Print performance statistics
 */

#ifndef PAIRING_HEAP_HPP
#define PAIRING_HEAP_HPP

#include <vector>
#include <stdexcept>
#include <chrono>
#include <iostream>

// Heap structure
struct HeapNode {
    int key;    // priority (in Dijkstra: distance, in Prim: weight)
    int value;  // vertex id

    HeapNode *parent;
    HeapNode *child; // leftmost child
    HeapNode *sibling; // next sibling

    HeapNode(int k, int v)
        : key(k), value(v),
          parent(nullptr),
          child(nullptr),
          sibling(nullptr) {}
       
    // Adds node as a child
    void addChild(HeapNode *node) { 
        node->parent = this;
        node->sibling = child;
        child = node;
    }
};

struct PairingHeap {
    HeapNode *root;
    
    // Performance tracking
    long long extract_min_time = 0;  // in microseconds
    long long decrease_key_time = 0; // in microseconds
    int insert_count = 0;
    int extract_min_count = 0;
    int decrease_key_count = 0;
    //int find_min_count = 0;

    PairingHeap() : root(nullptr) {}

    HeapNode* insert(int key, int value) {
        insert_count++;
        HeapNode* node = new HeapNode(key, value);
        root = merge(root, node);
        return node;
    }

    /*
    int find_min() {
        find_min_count++;
        if(!root) throw std::runtime_error("Heap is empty");
        return root->key;
    }
        */

    HeapNode* extract_min() {
        auto start = std::chrono::high_resolution_clock::now();
    
        if (!root) throw std::runtime_error("Heap is empty");

        HeapNode* old_root = root;        
        root = merge_pairs(root->child);  

        if (root) root->parent = nullptr; 

        auto end = std::chrono::high_resolution_clock::now();
        extract_min_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        extract_min_count++;

        return old_root;
    }

    void decrease_key(HeapNode* node, int new_key) {
        auto start = std::chrono::high_resolution_clock::now();
        
        if (new_key > node->key) {
            auto end = std::chrono::high_resolution_clock::now();
            decrease_key_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            decrease_key_count++;
            return;
        }
        
        node->key = new_key;

        if (node == root) {
            auto end = std::chrono::high_resolution_clock::now();
            decrease_key_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            decrease_key_count++;
            return;
        }

        cut(node);
        root = merge(root, node);
        
        auto end = std::chrono::high_resolution_clock::now();
        decrease_key_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        decrease_key_count++;
    }

    bool empty() {
        return root == nullptr;
    }

    void join(PairingHeap& other) {
        root = merge(root, other.root);
        other.root = nullptr;
    }

    // Print performance statistics
    void print_stats() {
        std::cout << "\n=== Pairing Heap Statistics ===\n";
        std::cout << "Number of operations:\n";
        std::cout << "  Insert:       " << insert_count << "\n";
        std::cout << "  Extract-min:  " << extract_min_count << "\n";
        std::cout << "  Decrease-key: " << decrease_key_count << "\n";
        //std::cout << "  Find-min:     " << find_min_count << "\n";
        std::cout << "\nTime spent:\n";
        std::cout << "  Extract-min:  " << extract_min_time / 1000.0 << " μs\n";
        std::cout << "  Decrease-key: " << decrease_key_time / 1000.0 << " μs\n";
        //std::cout << "  Avg extract:  " << (extract_min_count > 0 ? extract_min_time / (double)extract_min_count / 1000.0 : 0) << " μs\n";
        //std::cout << "  Avg decrease: " << (decrease_key_count > 0 ? decrease_key_time / (double)decrease_key_count / 1000.0 : 0) << " μs\n";
        std::cout << "==============================\n";
    }

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

    HeapNode* merge_pairs(HeapNode* first_child) {
        if (!first_child) return nullptr;
        if (!first_child->sibling) return first_child;

        std::vector<HeapNode*> trees;
        HeapNode* curr = first_child;

        while (curr) {
            HeapNode* a = curr;
            HeapNode* b = curr->sibling;

            if (b) {
                HeapNode* next = b->sibling;
                a->sibling = nullptr;
                b->sibling = nullptr;
                trees.push_back(merge(a, b));
                curr = next;
            } else {
                a->sibling = nullptr;
                trees.push_back(a);
                curr = nullptr;
            }
        }

        HeapNode* result = trees.back();
        for (int i = trees.size() - 2; i >= 0; i--) {
            result = merge(trees[i], result);
        }
        
        return result;
    }

    void cut(HeapNode* node) {
        if (!node->parent) return;

        if(node->parent->child == node) {
            node->parent->child = node->sibling;
        } else {
            HeapNode* prev = node->parent->child;
            while (prev->sibling != node) {
                prev = prev->sibling;
            }
            prev->sibling = node->sibling;
        }
        node->parent = nullptr;
        node->sibling = nullptr;
    }
};

#endif // PAIRING_HEAP_HPP
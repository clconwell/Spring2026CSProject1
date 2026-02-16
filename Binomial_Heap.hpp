#ifndef BINOMIAL_HEAP_HPP
#define BINOMIAL_HEAP_HPP

#include <vector>
#include <chrono>
#include <iostream>

class Binomial_Heap_Node{
    private:
        
    public:
        int vertexId;
        int key;
        Binomial_Heap_Node *parent;
        Binomial_Heap_Node * firstChild;
        int degree;
        Binomial_Heap_Node *next;
        Binomial_Heap_Node *sibling;

        Binomial_Heap_Node(int vertexId, int key){
            this->vertexId = vertexId;
            this->key = key;
            degree = 0;
            parent = nullptr;
            firstChild=nullptr;
            next = nullptr;
        }
};

class Binomial_Heap{
    private:
        Binomial_Heap_Node *head;
        Binomial_Heap_Node* min;

    public:
        // Performance tracking
        long long extract_min_time = 0;
        long long decrease_key_time = 0; 
        int insert_count = 0;
        int extract_min_count = 0;
        int decrease_key_count = 0;
        //int find_min_count = 0;

        Binomial_Heap(){
            head = nullptr;
            min = nullptr;
        }

        // initialize other operations.
        Binomial_Heap_Node* insert(int key, int vertexId){
            insert_count++;
            
            Binomial_Heap_Node* newNode = new Binomial_Heap_Node(vertexId, key);

            // add newNode as a root
            newNode->next = head;
            head = newNode;

            // merge trees of same degree
            linkSameDegreeTrees();

            // update min
            if (!min || newNode->key < min->key){
                min = newNode;
            }

            return newNode;
        }
        
        void merge(Binomial_Heap* heap){
            if (!heap || !heap->head) return; // nothing to merge

            Binomial_Heap_Node* h1 = head;
            Binomial_Heap_Node* h2 = heap->head;
            Binomial_Heap_Node* mergedHead = nullptr; // start of merged list
            Binomial_Heap_Node* tail = nullptr;       // last node in merged list

            // Merge root lists by degree
            while (h1 && h2) {
                Binomial_Heap_Node* nextNode = nullptr;
                if (h1->degree <= h2->degree) {
                    nextNode = h1;
                    h1 = h1->next;
                } else {
                    nextNode = h2;
                    h2 = h2->next;
                }

                // Append nextNode to merged list
                if (!mergedHead) {
                    mergedHead = nextNode;
                    tail = nextNode;
                } else {
                    tail->next = nextNode;
                    tail = nextNode;
                }
            }

            // Append remaining nodes
            while (h1) {
                if (!mergedHead) {
                    mergedHead = h1;
                    tail = h1;
                } else {
                    tail->next = h1;
                    tail = h1;
                }
                h1 = h1->next;
            }
            while (h2) {
                if (!mergedHead) {
                    mergedHead = h2;
                    tail = h2;
                } else {
                    tail->next = h2;
                    tail = h2;
                }
                h2 = h2->next;
            }

            // Make sure the last node points to nullptr
            if (tail) tail->next = nullptr;

            // Update head
            head = mergedHead;
        }

        void linkSameDegreeTrees() {
            if (!head) return;

            Binomial_Heap_Node* prev = nullptr;
            Binomial_Heap_Node* curr = head;
            Binomial_Heap_Node* next = curr->next;

            while (next) {
                // Case 1: degrees differ → move forward
                if (curr->degree != next->degree) {
                    prev = curr;
                    curr = next;
                }
                // Case 2: next-next has same degree → skip linking curr
                else if (next->next && next->next->degree == curr->degree) {
                    prev = curr;
                    curr = next;
                }
                // Case 3: degrees equal → LINK
                else {
                    // curr has smaller key → next becomes child
                    if (curr->key <= next->key) {
                        curr->next = next->next;

                        next->parent = curr;
                        next->sibling = curr->firstChild; // sibling points to old leftmost child
                        curr->firstChild = next;          // update leftmost child
                        curr->degree++;
                    }
                    // next has smaller key → curr becomes child
                    else {
                        if (prev == nullptr) {
                            head = next;
                        } else {
                            prev->next = next;
                        }

                        curr->parent = next;
                        curr->sibling = next->firstChild; // sibling points to old leftmost child
                        next->firstChild = curr;          // update leftmost child
                        next->degree++;

                        curr = next;
                    }
                }

                next = curr->next;
            }
        }

        /*
        Binomial_Heap_Node* find_min(){
            find_min_count++;
            return min;
        }
        */
        void decrease_key(Binomial_Heap_Node* node, int newKey){
            auto start = std::chrono::high_resolution_clock::now();
            
            node->key = newKey;
            Binomial_Heap_Node *parent = node->parent;

            while (parent && node->key < parent->key) {
                std::swap(parent->key, node->key);
                std::swap(parent->vertexId, node->vertexId);

                node = parent;
                parent = node->parent;
            }

            if (!min || node->key < min->key) {
                min = node;
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            decrease_key_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            decrease_key_count++;
        }
        
        int extract_min(){
            auto start = std::chrono::high_resolution_clock::now();
            
            if(!min) {
                auto end = std::chrono::high_resolution_clock::now();
                extract_min_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                extract_min_count++;
                return -1;
            }
            
            int minKey = min->key;
            //detach min tree
            Binomial_Heap_Node* prev = nullptr;
            Binomial_Heap_Node* curr = head;

            while (curr && curr != min) {
                prev = curr;
                curr = curr->next;
            }

            if (curr == min) {
                if (prev) {
                    prev->next = min->next;
                } else {
                    head = min->next;
                }
            }
            
            //detatch min tree children and add them to another heap
            Binomial_Heap_Node* subTreeHeap = nullptr;
            Binomial_Heap_Node* child = min->firstChild;
            min->firstChild = nullptr;

            while (child) {
                Binomial_Heap_Node* nextSibling = child->sibling;

                // Detach child
                child->parent = nullptr;
                child->sibling = nullptr;

                // PUSH TO FRONT (this reverses order)
                child->next = subTreeHeap;
                subTreeHeap = child;

                child = nextSibling;
            }
            Binomial_Heap subTreeHeapTemp;
            subTreeHeapTemp.head = subTreeHeap;
            merge(&subTreeHeapTemp);
            linkSameDegreeTrees();
            //delete subTreeHeapTemp;
            //update min pointer
            min = nullptr;
            Binomial_Heap_Node* cur = head;
            while (cur) {
                if (!min || cur->key < min->key) {
                    min = cur;
                }
                cur = cur->next;
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            extract_min_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            extract_min_count++;

            return minKey;
        }

        bool empty(){
            if(!head){
                return true;
            }
            else{
                return false;
            }
        }

        // Print performance statistics
        void print_stats() {
            std::cout << "\n=== Binomial Heap Statistics ===\n";
            std::cout << "Number of operations:\n";
            std::cout << "  Insert:       " << insert_count << "\n";
            std::cout << "  Extract-min:  " << extract_min_count << "\n";
            std::cout << "  Decrease-key: " << decrease_key_count << "\n";
            //std::cout << "  Find-min:     " << find_min_count << "\n";
            std::cout << "\nTime spent:\n";
            std::cout << "  Extract-min:  " << extract_min_time / 1000.0 << " μs\n";
            std::cout << "  Decrease-key: " << decrease_key_time / 1000.0 << " μs\n";
            //std::cout << "  Avg extract:  " << (extract_min_count > 0 ? extract_min_time / (double)extract_min_count / 1000.0 : 0) << " μs\n";
           // std::cout << "  Avg decrease: " << (decrease_key_count > 0 ? decrease_key_time / (double)decrease_key_count / 1000.0 : 0) << " μs\n";
            std::cout << "================================\n";
        }
};

#endif // BINOMIAL_HEAP_HPP
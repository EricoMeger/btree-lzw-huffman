#pragma once

#include "btree_types.hpp"
#include "../hash/algorithms/murmurhash.hpp"

#include <string>
#include <vector>
#include <cstdint>

class BTree {
public:
    struct Key {
        uint32_t hash = 0;
        std::string original;

        bool operator<(const Key& other) const {
            return hash != other.hash ? hash < other.hash : original < other.original;
        }
        bool operator==(const Key& other) const {
            return hash == other.hash && original == other.original;
        }
        bool operator>(const Key& other) const { 
            return other < *this; 
        }
        bool operator<=(const Key& other) const { 
            return !(other < *this); 
        }
        bool operator>=(const Key& other) const { 
            return !(*this < other);
        }
    };
    struct Node {
        int id;
        bool is_leaf;
        std::vector<Key> keys;
        std::vector<int> children; 

        Node(int id, bool is_leaf) : id(id), is_leaf(is_leaf) {}
    };

    explicit BTree(int order);
    
    OpResult insert(const std::string& key);
    OpResult read(const std::string& key) const;
    OpResult remove(const std::string& key);

    bool save(const std::string& filename) const;
    bool load(const std::string& filename);

    int get_order() const { return order; }
    int node_count() const { return static_cast<int>(nodes.size());}
    bool empty() const { return root_id == -1; }
    void print() const;

private:
    int order;
    int root_id;
    int next_id;
    std::vector<Node> nodes;
    MurmurHash hasher;

    Node& node(int id) { return nodes[id]; }
    const Node& node(int id) const { return nodes[id]; }
    int  min_keys()  const;   // grau min t-1, t=ordem/2
    int  max_keys()  const;   // 2t-1
    int  split_mid() const;  
    Key  make_key(const std::string& s);
    int  new_node(bool is_leaf);    // aloca nó e retorna seu id
    
    // ---- insert
    void split_child(int parent_id, int child_pos);
    void insert_non_full(int node_id, const Key& key, int& visited);
    
    // ----- search
    bool search(int node_id, const Key& key, int& visited) const;

    // ----- delete
    bool remove_from(int node_id, const Key& key, int& visited);
    void remove_from_leaf(int node_id, int idx);
    void remove_from_internal(int node_id, int idx, int& visited);
    Key  predecessor(int node_id);   // maior chave da sub-árvore
    Key  successor  (int node_id);   // menor chave da sub-árvore
    void ensure_min (int parent_id, int child_pos);
    void borrow_prev(int parent_id, int child_pos);
    void borrow_next(int parent_id, int child_pos);
    void merge_child(int parent_id, int child_pos);   // merge children[pos] e [pos+1]

    // ----- salvar no arquivo
    void write_node(std::ostream& out, const Node& n) const;

};
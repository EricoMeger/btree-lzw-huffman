#include "../../include/btree/btree.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iomanip>

BTree::BTree(int order) : order(order), root_id(-1), next_id(0), hasher(0) {
    if (order < 3)
        throw std::invalid_argument("Btree has a minimum order of 3");
}

#pragma region HELPERS
int BTree::min_keys() const { 
    return ((order + 1) / 2) - 1; 
}
int BTree::max_keys() const { 
    return 2 * ((order + 1) / 2) - 1; 
}
int BTree::split_mid() const { 
    return ((order + 1) / 2) - 1; 
} // índice da chave promovida

BTree::Key BTree::make_key(const std::string& content) {
    Key k;
    k.hash = static_cast<uint32_t>(hasher.hash(content));
    k.original = content;
    return k;
}

#pragma endregion HELPERS

int BTree::new_node(bool is_leaf) {
    int id = next_id++;
    nodes.emplace_back(id, is_leaf);
    return id;
}

#pragma region INSERT

OpResult BTree::insert(const std::string& key) {
    int visited = 0;
    Key k = make_key(key);

    //creates root if tree is empty
    if (root_id == -1) {
        root_id = new_node(true);
        nodes[root_id].keys.push_back(k);
        visited = 1;
        return {true, visited};
    }

    //searches for duplicates
    int dup_visited = 0;
    if (search(root_id, k, dup_visited)) {
        return {false, dup_visited};
    }
    
    //if the root is full, split it and create a new root
    if (static_cast<int>(nodes[root_id].keys.size()) == max_keys()) {
        int old_root = root_id;
        root_id = new_node(false);
        nodes[root_id].children.push_back(old_root);
        split_child(root_id, 0);
    }

    insert_non_full(root_id, k, visited);
    return {true, visited};
}

//children needs to be full
void BTree::split_child(int parent_id, int child_pos) {
    int left_id = nodes[parent_id].children[child_pos];
    int right_id = new_node(nodes[left_id].is_leaf); 

    int mid = split_mid();

    Key promoted = nodes[left_id].keys[mid];

    nodes[right_id].keys.assign(
        nodes[left_id].keys.begin() + mid + 1,
        nodes[left_id].keys.end());

    if (!nodes[left_id].is_leaf) {
        nodes[right_id].children.assign(
            nodes[left_id].children.begin() + mid + 1,
            nodes[left_id].children.end());
        nodes[left_id].children.resize(mid + 1);
    }

    nodes[left_id].keys.resize(mid);

    nodes[parent_id].keys.insert(
        nodes[parent_id].keys.begin() + child_pos, promoted);
    nodes[parent_id].children.insert(
        nodes[parent_id].children.begin() + child_pos + 1, right_id);
}

void BTree::insert_non_full(int node_id, const Key& key, int& visited) {
    visited++;

    if (nodes[node_id].is_leaf) {
        auto& keys = nodes[node_id].keys;
        auto it = std::lower_bound(keys.begin(), keys.end(), key);
        keys.insert(it, key);
        return;
    }

    auto& keys = nodes[node_id].keys;
    auto it = std::lower_bound(keys.begin(), keys.end(), key);
    int ci = static_cast<int>(it - keys.begin());
    int child_id = nodes[node_id].children[ci];

    if (static_cast<int>(nodes[child_id].keys.size()) == max_keys()) {
        split_child(node_id, ci);
        if (key > nodes[node_id].keys[ci])
            child_id = nodes[node_id].children[ci + 1];
        else
            child_id = nodes[node_id].children[ci];
    }

    insert_non_full(child_id, key, visited);
}

#pragma endregion INSERT

#pragma region SEARCH

OpResult BTree::read(const std::string& key) const {
    int visited = 0;
    if (root_id == -1) {
        return {false, 0};
    }
    Key k = const_cast<BTree*>(this)->make_key(key);

    return {search(root_id, k, visited), visited};
}

bool BTree::search(int node_id, const Key& key, int& visited) const {
    visited++;
    const auto& keys = nodes[node_id].keys;
    auto it = std::lower_bound(keys.begin(), keys.end(), key);

    if (it != keys.end() && *it == key) return true;
    if (nodes[node_id].is_leaf) return false;

    int ci = static_cast<int>(it - keys.begin());
    return search(nodes[node_id].children[ci], key, visited);
}

#pragma endregion SEARCH

#pragma region DELETE

OpResult BTree::remove(const std::string& key) {
    int visited = 0;
    if (root_id == -1) return {false, 0};
    Key k = make_key(key);
    bool removed = remove_from(root_id, k, visited);

    //if the root got empty and is not a leaf, make the first child the new root
    if (!nodes[root_id].is_leaf && nodes[root_id].keys.empty())
        root_id = nodes[root_id].children[0];

    return {removed, visited};
}

bool BTree::remove_from(int node_id, const Key& key, int& visited) {
    visited++;
    auto& keys = nodes[node_id].keys;
    auto it = std::lower_bound(keys.begin(), keys.end(), key);
    int idx = static_cast<int>(it - keys.begin());

    if (it != keys.end() && *it == key) {
        if (nodes[node_id].is_leaf)
            remove_from_leaf(node_id, idx);
        else
            remove_from_internal(node_id, idx, visited);
        return true;
    }

    if (nodes[node_id].is_leaf) return false;

    int child_id = nodes[node_id].children[idx];

    if (static_cast<int>(nodes[child_id].keys.size()) <= min_keys()) {
        ensure_min(node_id, idx);

        auto& keys2 = nodes[node_id].keys;
        auto it2 = std::lower_bound(keys2.begin(), keys2.end(), key);
        idx = static_cast<int>(it2 - keys2.begin());

        if (it2 != keys2.end() && *it2 == key) {
            if (nodes[node_id].is_leaf)
                remove_from_leaf(node_id, idx);
            else
                remove_from_internal(node_id, idx, visited);
            return true;
        }

        child_id = nodes[node_id].children[idx];
    }

    return remove_from(child_id, key, visited);
}

void BTree::remove_from_leaf(int node_id, int idx) {
    nodes[node_id].keys.erase(nodes[node_id].keys.begin() + idx);
}

void BTree::remove_from_internal(int node_id, int idx, int& visited) {
    int left_id = nodes[node_id].children[idx];
    int right_id = nodes[node_id].children[idx + 1];

    if (static_cast<int>(nodes[left_id].keys.size()) > min_keys()) {
        Key pred = predecessor(left_id);
        nodes[node_id].keys[idx] = pred;
        remove_from(left_id, pred, visited);
    } else if (static_cast<int>(nodes[right_id].keys.size()) > min_keys()) {
        Key succ = successor(right_id);
        nodes[node_id].keys[idx] = succ;
        remove_from(right_id, succ, visited);
    } else {
        merge_child(node_id, idx);
        remove_from(left_id, nodes[node_id].keys[idx], visited);
    }
}

BTree::Key BTree::predecessor(int node_id) {
    while (!nodes[node_id].is_leaf)
        node_id = nodes[node_id].children.back();
    return nodes[node_id].keys.back();
}

BTree::Key BTree::successor(int node_id) {
    while (!nodes[node_id].is_leaf)
        node_id = nodes[node_id].children.front();
    return nodes[node_id].keys.front();
}

void BTree::ensure_min(int parent_id, int child_pos) {
    int n_children = static_cast<int>(nodes[parent_id].children.size());

    if (child_pos > 0 &&
        static_cast<int>(nodes[nodes[parent_id].children[child_pos - 1]].keys.size()) > min_keys()) {
        borrow_prev(parent_id, child_pos);
    } else if (child_pos < n_children - 1 && static_cast<int>(nodes[nodes[parent_id].children[child_pos + 1]].keys.size()) > min_keys()) {
        borrow_next(parent_id, child_pos);
    } else {
        if (child_pos < n_children - 1)
            merge_child(parent_id, child_pos);
        else
            merge_child(parent_id, child_pos - 1);
    }
}

void BTree::borrow_prev(int parent_id, int child_pos) {
    int child_id = nodes[parent_id].children[child_pos];
    int sibling_id = nodes[parent_id].children[child_pos - 1];

    nodes[child_id].keys.insert(
        nodes[child_id].keys.begin(),
        nodes[parent_id].keys[child_pos - 1]);

    if (!nodes[child_id].is_leaf) {
        nodes[child_id].children.insert(
            nodes[child_id].children.begin(),
            nodes[sibling_id].children.back());
        nodes[sibling_id].children.pop_back();
    }

    nodes[parent_id].keys[child_pos - 1] = nodes[sibling_id].keys.back();
    nodes[sibling_id].keys.pop_back();
}

void BTree::borrow_next(int parent_id, int child_pos) {
    int child_id = nodes[parent_id].children[child_pos];
    int sibling_id = nodes[parent_id].children[child_pos + 1];

    nodes[child_id].keys.push_back(nodes[parent_id].keys[child_pos]);

    if (!nodes[child_id].is_leaf) {
        nodes[child_id].children.push_back(nodes[sibling_id].children.front());
        nodes[sibling_id].children.erase(nodes[sibling_id].children.begin());
    }

    nodes[parent_id].keys[child_pos] = nodes[sibling_id].keys.front();
    nodes[sibling_id].keys.erase(nodes[sibling_id].keys.begin());
}

void BTree::merge_child(int parent_id, int child_pos) {
    int left_id  = nodes[parent_id].children[child_pos];
    int right_id = nodes[parent_id].children[child_pos + 1];

    nodes[left_id].keys.push_back(nodes[parent_id].keys[child_pos]);

    for (auto& k : nodes[right_id].keys)
        nodes[left_id].keys.push_back(k);
    if (!nodes[left_id].is_leaf)
        for (int c : nodes[right_id].children)
            nodes[left_id].children.push_back(c);

    nodes[parent_id].keys.erase(nodes[parent_id].keys.begin() + child_pos);
    nodes[parent_id].children.erase(nodes[parent_id].children.begin() + child_pos + 1);
}

#pragma endregion DELETE

#pragma region SAVE

bool BTree::save(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    out << "BTREE order=" << order << " nodes=" << nodes.size() << " root=" << root_id << "\n\n";

    for (const auto& n : nodes) {
        write_node(out, n);
        out << "\n";
    }
    return true;
}

void BTree::write_node(std::ostream& out, const Node& n) const {
    //temporario para testes somente, dps fazer puxar a compressao antes de salvar
    
    out << "[" << n.id << "] ";
    if (n.id == root_id) out << "ROOT ";
    out << (n.is_leaf ? "LEAF" : "INTERNAL");
    out << " keys=" << n.keys.size();
    if (!n.is_leaf) out << " children=" << n.children.size();
    out << "\n";

    out << "keys:";
    for (const auto& k : n.keys)
        out << " " << std::hex << std::setw(8) << std::setfill('0') << k.hash
            << ":" << k.original;
    out << "\n";

    if (!n.is_leaf) {
        out << "children:";
        for (int c : n.children) out << " " << std::dec << c;
        out << "\n";
    }
}

#pragma endregion SAVE

#pragma region LOAD

bool BTree::load(const std::string& filename) {
    return true;

    // TODO
}

#pragma endregion LOAD

void BTree::print() const {
    if (root_id == -1) { std::cout << "(tree is empty)\n"; return; }
    for (const auto& n : nodes) {
        std::cout << "[" << n.id << "] "
                  << (n.id == root_id ? "ROOT " : "")
                  << (n.is_leaf ? "LEAF" : "INTERNAL")
                  << " keys=" << n.keys.size() << "\n";
        std::cout << "  keys:";
        for (const auto& k : n.keys) std::cout << " " << k.original;
        std::cout << "\n";
        if (!n.is_leaf) {
            std::cout << "  children:";
            for (int c : n.children) std::cout << " " << c;
            std::cout << "\n";
        }
    }
}

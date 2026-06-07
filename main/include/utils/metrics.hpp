#pragma once

#include <chrono>
#include <string>
#include <cstddef>

struct Metrics {
    int         nodes_visited;   
    double      elapsed_ms;      
    std::size_t memory_bytes;    
};
class Timer {
public:
    Timer();
    double elapsed_ms() const; 
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

std::size_t current_memory_bytes();

void print_metrics(const std::string& op, const std::string& key, const Metrics& m);

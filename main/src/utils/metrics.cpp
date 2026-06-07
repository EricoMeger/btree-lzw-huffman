#include "../../include/utils/metrics.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>

Timer::Timer() : start_(std::chrono::high_resolution_clock::now()) {}

double Timer::elapsed_ms() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(now - start_).count();
}

//only works on linux, reads VmRSS from /proc/self/status
// based on https://stackoverflow.com/a/9513713
std::size_t current_memory_bytes() {
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            std::size_t kb = 0;
            std::istringstream ss(line.substr(6));
            ss >> kb;
            return kb;
        }
    }
    return 0; 
}

void print_metrics(const std::string& op, const std::string& key, const Metrics& m) {
    std::cout << "[" << op << "] key=\"" << key << "\""
              << " | nodes_visited=" << m.nodes_visited
              << " | time=" << std::fixed << std::setprecision(3) << m.elapsed_ms << "ms"
              << " | memory=" << m.memory_bytes << "KB"
              << "\n";
}

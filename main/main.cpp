#include "include/cli/orchestrator.hpp"

#include <iostream>
#include <string>
#include <cstdlib>

static void usage(const char* prog) {
    std::cerr << "Usage: " << prog << " --order <N>\n"
              << "  --order <N>   B-Tree order (minimum 3)\n"
              << "  --help        displays this message\n";
}

int main(int argc, char* argv[]) {
    int order = 0;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--order" && i + 1 < argc) {
            order = std::atoi(argv[++i]);
        } else if (arg == "--help") {
            usage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            usage(argv[0]);
            return 1;
        }
    }

    if (order < 3) {
        std::cerr << "ERROR: order must be >= 3. Use --order <N>.\n";
        usage(argv[0]);
        return 1;
    }

    std::cout << "treectl -- B-Tree (order " << order << ")\n"
              << "Type \"help\" to see the available commands.\n\n";

    Orchestrator orc(order);
    std::string  line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;   
        if (!orc.handle(line)) break;               
    }

    return 0;
}

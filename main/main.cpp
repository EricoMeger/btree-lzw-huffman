#include "include/cli/orchestrator.hpp"

#include <iostream>
#include <string>
#include <cstdlib>
#include <memory>

#ifdef WITH_COMPRESSION
#include "compression/huffman.hpp"
#include "compression/lzw.hpp"
#include "decompression/huffman.hpp"
#include "decompression/lzw.hpp"
#endif

static void usage(const char* prog) {
    std::cerr << "Usage: " << prog << " --order <N> [--compressor <huffman|lzw>]\n"
              << "  --order <N> B-Tree order (minimum 3)\n"
#ifdef WITH_COMPRESSION
              << "  --compressor <algorithm>  compress on save / decompress on load\n"
#endif
              << "  --help displays this message\n";
}

int main(int argc, char* argv[]) {
    int order = 0;
    std::string compressor_name;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--order" && i + 1 < argc) {
            order = std::atoi(argv[++i]);
        } else if (arg == "--compressor" && i + 1 < argc) {
            compressor_name = argv[++i];
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

    std::cout << "treectl -- B-Tree (order " << order << ")\n";

    Orchestrator orc(order);

#ifdef WITH_COMPRESSION
    if (!compressor_name.empty()) {
        std::unique_ptr<Compressor> comp;
        std::unique_ptr<Decompressor> decomp;

        if (compressor_name == "huffman") {
            comp = std::make_unique<HuffmanCompressor>();
            decomp = std::make_unique<HuffmanDecompressor>();
        } else if (compressor_name == "lzw") {
            comp = std::make_unique<LZWCompressor>();
            decomp = std::make_unique<LZWDecompressor>();
        } else {
            std::cerr << "ERROR: unknown compressor \"" << compressor_name
                      << "\". Use huffman or lzw.\n";
            return 1;
        }

        std::cout << "Compression: " << compressor_name << "\n";
        orc.set_compressor(std::move(comp), std::move(decomp));
    }
#else
    if (!compressor_name.empty()) {
        std::cerr << "WARN: this build was compiled without compression support.\n";
    }
#endif

    std::cout << "Type \"help\" to see the available commands.\n\n";

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (!orc.handle(line)) break;
    }

    return 0;
}

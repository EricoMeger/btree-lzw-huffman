#include "../../include/cli/orchestrator.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

#ifdef WITH_COMPRESSION
#include <vector>
#endif

Orchestrator::Orchestrator(int order) : tree(order) {}

#ifdef WITH_COMPRESSION
void Orchestrator::set_compressor(std::unique_ptr<Compressor> comp, std::unique_ptr<Decompressor> decomp) {
    compressor   = std::move(comp);
    decompressor = std::move(decomp);
}

static std::vector<uint8_t> read_bytes(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Cannot open: " + path);
    } 
    auto size = file.tellg();
    file.seekg(0);
    std::vector<uint8_t> buf(size);
    file.read(reinterpret_cast<char*>(buf.data()), size);
    return buf;
}

static void write_bytes(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot write: " + path);
    } 
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}
#endif

bool Orchestrator::handle(const std::string& line) {
    std::istringstream ss(line);
    std::string cmd;
    ss >> cmd;

    if (cmd.empty() || cmd[0] == '#') return true;
    if (cmd == "exit" || cmd == "quit") return false;

    std::string arg;
    ss >> arg;

    if (cmd == "insert") cmd_insert(arg);
    else if (cmd == "read")   cmd_read(arg);
    else if (cmd == "delete") cmd_delete(arg);
    else if (cmd == "save")   cmd_save(arg);
    else if (cmd == "load")   cmd_load(arg);
    else if (cmd == "print")  cmd_print();
    else if (cmd == "help")   cmd_help();
    else {
        std::cout << "Unrecognized command \"" << cmd
                  << "\". Type \"help\" to see the available commands.\n";
    }

    return true;
}

void Orchestrator::cmd_insert(const std::string& key) {
    if (key.empty()) { std::cout << "Usage: insert <key>\n"; return; }
    Timer t;
    auto result = tree.insert(key);
    Metrics m{result.nodes_visited, t.elapsed_ms(), current_memory_bytes()};
    std::cout << (result.found ? "OK inserted: \"" : "WARN: key \"") << key
              << (result.found ? "\"\n" : "\" already exists.\n");
    print_metrics("insert", key, m);
}

void Orchestrator::cmd_read(const std::string& key) {
    if (key.empty()) { std::cout << "Usage: read <key>\n"; return; }
    Timer t;
    auto result = tree.read(key);
    Metrics m{result.nodes_visited, t.elapsed_ms(), current_memory_bytes()};
    std::cout << (result.found ? "FOUND: \"" : "NOT FOUND: \"") << key << "\"\n";
    print_metrics("read", key, m);
}

void Orchestrator::cmd_delete(const std::string& key) {
    if (key.empty()) { std::cout << "Usage: delete <key>\n"; return; }
    Timer t;
    auto result = tree.remove(key);
    Metrics m{result.nodes_visited, t.elapsed_ms(), current_memory_bytes()};
    std::cout << (result.found ? "OK removed: \"" : "NOT FOUND: \"") << key << "\"\n";
    print_metrics("delete", key, m);
}

void Orchestrator::cmd_save(const std::string& filename) {
    if (filename.empty()) { std::cout << "Usage: save <file>\n"; return; }

    Timer t;
    bool ok = tree.save(filename);
    if (!ok) { std::cout << "ERROR: Could not save to \"" << filename << "\"\n"; return; }

#ifdef WITH_COMPRESSION
    if (compressor) {
        try {
            auto raw = read_bytes(filename);
            auto compressed = compressor->compress(raw);
            std::string out = filename + "." + compressor->name();
            write_bytes(out, compressed);
            double ratio = compressed.size() * 100.0 / raw.size();
            std::cout << "Tree saved to \"" << filename << "\" and compressed to \""
                      << out << "\" (" << std::fixed << ratio << "% of original, "
                      << t.elapsed_ms() << "ms)\n";
            return;
        } catch (const std::exception& e) {
            std::cout << "WARN: compression failed (" << e.what() << "), plain file kept.\n";
        }
    }
#endif

    std::cout << "Tree saved to \"" << filename << "\" (" << t.elapsed_ms() << "ms)\n";
}

void Orchestrator::cmd_load(const std::string& filename) {
    if (filename.empty()) { std::cout << "Usage: load <file>\n"; return; }

#ifdef WITH_COMPRESSION
    if (decompressor) {
        std::string compressed_path = filename + "." + decompressor->name();
        std::ifstream probe(compressed_path);
        if (probe.good()) {
            probe.close();
            try {
                Timer t;
                auto raw = read_bytes(compressed_path);
                auto decompressed = decompressor->decompress(raw);
                write_bytes(filename, decompressed);   // restaura o .txt para load normal
                std::cout << "Decompressed \"" << compressed_path
                          << "\" -> \"" << filename << "\"\n";
            } catch (const std::exception& e) {
                std::cout << "WARN: decompression failed (" << e.what() << "), trying plain file.\n";
            }
        }
    }
#endif

    Timer t;
    bool ok = tree.load(filename);
    if (ok)
        std::cout << "Tree loaded from \"" << filename << "\" ("
                  << tree.node_count() << " nodes, " << t.elapsed_ms() << "ms)\n";
    else
        std::cout << "ERROR: Could not load from \"" << filename << "\"\n";
}

void Orchestrator::cmd_print() {
    tree.print();
}

void Orchestrator::cmd_help() {
    std::cout <<
        "Available commands:\n"
        "  insert <key>   inserts the key into the tree\n"
        "  read   <key>   reads the key from the tree\n"
        "  delete <key>   deletes the key from the tree\n"
        "  save   <file>  saves the tree to a file\n"
        "  load   <file>  loads the tree from a file\n"
        "  print          prints the tree to the terminal\n"
        "  help           displays this help message\n"
        "  exit           exits the program\n";
}

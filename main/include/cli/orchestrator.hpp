#pragma once

#include "../btree/btree.hpp"
#include "../utils/metrics.hpp"
#include <string>
#include <memory>

#ifdef WITH_COMPRESSION
#include "compressor.hpp"
#endif

class Orchestrator {
public:
    explicit Orchestrator(int order);

#ifdef WITH_COMPRESSION
    // Define qual compressor usar no save/load.
    // nullptr = sem compressão (standard).
    void set_compressor(std::unique_ptr<Compressor> comp,
                        std::unique_ptr<Decompressor> decomp);
#endif

    bool handle(const std::string& line);   // false = exit

private:
    BTree tree;

#ifdef WITH_COMPRESSION
    std::unique_ptr<Compressor>   compressor;
    std::unique_ptr<Decompressor> decompressor;
#endif

    void cmd_insert(const std::string& key);
    void cmd_read  (const std::string& key);
    void cmd_delete(const std::string& key);
    void cmd_save  (const std::string& filename);
    void cmd_load  (const std::string& filename);
    void cmd_print ();
    void cmd_help  ();
};

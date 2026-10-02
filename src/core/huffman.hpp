#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace gcore {

struct HuffmanNode {
    uint8_t symbol = 0;
    uint32_t frequency = 0;
    uint64_t order = 0;
    bool is_leaf = false;
    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;
};

using HuffmanTree = std::shared_ptr<HuffmanNode>;

/// Builds Huffman trees and encodes/decodes byte streams.
class HuffmanCodec {
public:
    /// Compresses raw bytes into a self-contained Huffman bitstream.
    std::vector<uint8_t> compress(const std::vector<uint8_t>& input) const;

    /// Decompresses bytes produced by compress().
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& input) const;

private:
    HuffmanTree build_tree(const std::unordered_map<uint8_t, uint32_t>& frequencies) const;
    void fill_table(const HuffmanTree& node, const std::string& prefix,
                    std::unordered_map<uint8_t, std::string>& table) const;
    std::unordered_map<uint8_t, uint32_t> count_frequencies(
        const std::vector<uint8_t>& input) const;
};

}  // namespace gcore

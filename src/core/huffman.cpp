#include "huffman.hpp"

#include "bitstream.hpp"

#include <algorithm>
#include <map>
#include <queue>
#include <stdexcept>

namespace gcore {

namespace {

struct NodeCompare {
    bool operator()(const HuffmanTree& a, const HuffmanTree& b) const {
        if (a->frequency != b->frequency) {
            return a->frequency > b->frequency;
        }
        return a->order > b->order;
    }
};

}  // namespace

std::unordered_map<uint8_t, uint32_t> HuffmanCodec::count_frequencies(
    const std::vector<uint8_t>& input) const {
    std::unordered_map<uint8_t, uint32_t> frequencies;
    for (uint8_t byte : input) {
        ++frequencies[byte];
    }
    return frequencies;
}

HuffmanTree HuffmanCodec::build_tree(
    const std::unordered_map<uint8_t, uint32_t>& frequencies) const {
    if (frequencies.empty()) {
        return nullptr;
    }

    std::priority_queue<HuffmanTree, std::vector<HuffmanTree>, NodeCompare> heap;
    for (const auto& entry : frequencies) {
        auto node = std::make_shared<HuffmanNode>();
        node->symbol = entry.first;
        node->frequency = entry.second;
        node->order = entry.first;
        node->is_leaf = true;
        heap.push(node);
    }

    // Single-symbol input: wrap in dummy parent so every code has at least one bit.
    if (heap.size() == 1) {
        auto wrapper = std::make_shared<HuffmanNode>();
        wrapper->frequency = heap.top()->frequency;
        wrapper->is_leaf = false;
        wrapper->left = heap.top();
        wrapper->right = heap.top();
        heap.pop();
        heap.push(wrapper);
    }

    while (heap.size() > 1) {
        auto left = heap.top();
        heap.pop();
        auto right = heap.top();
        heap.pop();

        auto parent = std::make_shared<HuffmanNode>();
        parent->frequency = left->frequency + right->frequency;
        parent->order = std::min(left->order, right->order);
        parent->is_leaf = false;
        parent->left = left;
        parent->right = right;
        heap.push(parent);
    }

    return heap.top();
}

void HuffmanCodec::fill_table(const HuffmanTree& node, const std::string& prefix,
                              std::unordered_map<uint8_t, std::string>& table) const {
    if (!node) {
        return;
    }
    if (node->is_leaf) {
        table[node->symbol] = prefix.empty() ? "0" : prefix;
        return;
    }
    fill_table(node->left, prefix + '0', table);
    fill_table(node->right, prefix + '1', table);
}

std::vector<uint8_t> HuffmanCodec::compress(const std::vector<uint8_t>& input) const {
    BitWriter writer;

    if (input.empty()) {
        writer.write_uint32(0);
        return writer.finish();
    }

    const auto frequencies = count_frequencies(input);
    const HuffmanTree tree = build_tree(frequencies);

    std::unordered_map<uint8_t, std::string> codes;
    fill_table(tree, "", codes);

    writer.write_uint32(static_cast<uint32_t>(input.size()));

    const uint32_t unique_count = static_cast<uint32_t>(frequencies.size());
    writer.write_uint32(unique_count);

    std::map<uint8_t, uint32_t> sorted_freq(frequencies.begin(), frequencies.end());
    for (const auto& entry : sorted_freq) {
        writer.write_uint8(entry.first);
        writer.write_uint32(entry.second);
    }

    for (uint8_t byte : input) {
        const std::string& code = codes.at(byte);
        for (char bit : code) {
            writer.write_bit(bit == '1');
        }
    }

    return writer.finish();
}

std::vector<uint8_t> HuffmanCodec::decompress(const std::vector<uint8_t>& input) const {
    BitReader reader(input);
    const uint32_t original_size = reader.read_uint32();
    if (original_size == 0) {
        return {};
    }

    const uint32_t unique_count = reader.read_uint32();
    std::unordered_map<uint8_t, uint32_t> frequencies;
    for (uint32_t i = 0; i < unique_count; ++i) {
        const uint8_t symbol = reader.read_uint8();
        const uint32_t freq = reader.read_uint32();
        frequencies[symbol] = freq;
    }

    const HuffmanTree tree = build_tree(frequencies);
    std::vector<uint8_t> output;
    output.reserve(original_size);

    HuffmanTree node = tree;
    while (output.size() < original_size) {
        if (!node) {
            throw std::runtime_error("HuffmanCodec: invalid tree during decode");
        }
        if (node->is_leaf) {
            output.push_back(node->symbol);
            node = tree;
            continue;
        }
        const bool bit = reader.read_bit();
        node = bit ? node->right : node->left;
    }

    return output;
}

}  // namespace gcore

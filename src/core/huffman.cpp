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
        if (a->frequency != b->frequency) return a->frequency > b->frequency;
        return a->order > b->order;
    }
};

}  // namespace

std::unordered_map<uint8_t, uint32_t>
HuffmanCodec::count_frequencies(const std::vector<uint8_t>& input) const {
    std::unordered_map<uint8_t, uint32_t> freq;
    freq.reserve(256);
    for (uint8_t b : input) ++freq[b];
    return freq;
}

HuffmanTree HuffmanCodec::build_tree(
    const std::unordered_map<uint8_t, uint32_t>& frequencies) const {
    if (frequencies.empty()) return nullptr;

    std::priority_queue<HuffmanTree, std::vector<HuffmanTree>, NodeCompare> heap;
    for (const auto& e : frequencies) {
        auto node       = std::make_shared<HuffmanNode>();
        node->symbol    = e.first;
        node->frequency = e.second;
        node->order     = e.first;
        node->is_leaf   = true;
        heap.push(node);
    }

    if (heap.size() == 1) {
        auto w       = std::make_shared<HuffmanNode>();
        w->frequency = heap.top()->frequency;
        w->is_leaf   = false;
        w->left      = heap.top();
        w->right     = heap.top();
        heap.pop();
        heap.push(w);
    }

    while (heap.size() > 1) {
        auto left  = heap.top(); heap.pop();
        auto right = heap.top(); heap.pop();
        auto par        = std::make_shared<HuffmanNode>();
        par->frequency  = left->frequency + right->frequency;
        par->order      = std::min(left->order, right->order);
        par->is_leaf    = false;
        par->left       = left;
        par->right      = right;
        heap.push(par);
    }

    return heap.top();
}

// ---------------------------------------------------------------------------
// fill_table() – builds integer-coded lookup table instead of strings.
// Each entry is {bit_pattern (MSB-aligned in a uint32), bit_length}.
// This avoids per-bit string iteration during encoding.
// ---------------------------------------------------------------------------
void HuffmanCodec::fill_table(const HuffmanTree& node, const std::string& prefix,
                               std::unordered_map<uint8_t, std::string>& table) const {
    if (!node) return;
    if (node->is_leaf) {
        table[node->symbol] = prefix.empty() ? "0" : prefix;
        return;
    }
    fill_table(node->left,  prefix + '0', table);
    fill_table(node->right, prefix + '1', table);
}

// Internal: fills integer-coded table for fast encoding.
static void fill_int_table(const HuffmanTree& node, uint32_t code, int depth,
                            std::unordered_map<uint8_t, std::pair<uint32_t,int>>& table) {
    if (!node) return;
    if (node->is_leaf) {
        int len = depth == 0 ? 1 : depth;
        uint32_t pat = depth == 0 ? 0u : code;
        table[node->symbol] = {pat, len};
        return;
    }
    fill_int_table(node->left,  (code << 1),      depth + 1, table);
    fill_int_table(node->right, (code << 1) | 1u, depth + 1, table);
}

// ---------------------------------------------------------------------------
// compress()
// ---------------------------------------------------------------------------
std::vector<uint8_t> HuffmanCodec::compress(const std::vector<uint8_t>& input) const {
    BitWriter writer;

    if (input.empty()) {
        writer.write_uint32(0);
        return writer.finish();
    }

    const auto frequencies = count_frequencies(input);
    const HuffmanTree tree = build_tree(frequencies);

    // Build integer-coded table for fast bit emission.
    std::unordered_map<uint8_t, std::pair<uint32_t,int>> int_codes;
    int_codes.reserve(256);
    fill_int_table(tree, 0u, 0, int_codes);

    // Header: original size + frequency table.
    writer.write_uint32(static_cast<uint32_t>(input.size()));
    writer.write_uint32(static_cast<uint32_t>(frequencies.size()));

    std::map<uint8_t, uint32_t> sorted_freq(frequencies.begin(), frequencies.end());
    for (const auto& e : sorted_freq) {
        writer.write_uint8(e.first);
        writer.write_uint32(e.second);
    }

    // Encode: emit bits using integer codes (no per-char string traversal).
    for (uint8_t byte : input) {
        const auto& [pat, len] = int_codes.at(byte);
        for (int i = len - 1; i >= 0; --i)
            writer.write_bit(((pat >> i) & 1u) != 0u);
    }

    return writer.finish();
}

// ---------------------------------------------------------------------------
// decompress()
// ---------------------------------------------------------------------------
std::vector<uint8_t> HuffmanCodec::decompress(const std::vector<uint8_t>& input) const {
    BitReader reader(input);
    const uint32_t original_size = reader.read_uint32();
    if (original_size == 0) return {};

    const uint32_t unique_count = reader.read_uint32();
    std::unordered_map<uint8_t, uint32_t> frequencies;
    frequencies.reserve(unique_count);
    for (uint32_t i = 0; i < unique_count; ++i) {
        const uint8_t sym  = reader.read_uint8();
        const uint32_t freq = reader.read_uint32();
        frequencies[sym] = freq;
    }

    const HuffmanTree tree = build_tree(frequencies);
    std::vector<uint8_t> output;
    output.reserve(original_size);

    HuffmanTree node = tree;
    while (output.size() < original_size) {
        if (!node) throw std::runtime_error("HuffmanCodec: invalid tree during decode");
        if (node->is_leaf) {
            output.push_back(node->symbol);
            node = tree;
            continue;
        }
        node = reader.read_bit() ? node->right : node->left;
    }

    return output;
}

}  // namespace gcore

#include "core/huffman.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

bool round_trip(const std::vector<uint8_t>& input) {
    gcore::HuffmanCodec codec;
    const std::vector<uint8_t> compressed = codec.compress(input);
    const std::vector<uint8_t> restored = codec.decompress(compressed);
    return restored == input;
}

}  // namespace

int main() {
    const std::string text = "this is a test";
    const std::vector<uint8_t> sample(text.begin(), text.end());
    if (!round_trip(sample)) {
        std::cerr << "[FAIL] Huffman text round-trip\n";
        return 1;
    }

    std::vector<uint8_t> repeated(1000, 0x42);
    if (!round_trip(repeated)) {
        std::cerr << "[FAIL] Huffman repeated round-trip\n";
        return 1;
    }

    if (!round_trip({})) {
        std::cerr << "[FAIL] Huffman empty round-trip\n";
        return 1;
    }

    std::cout << "[PASS] Huffman\n";
    return 0;
}

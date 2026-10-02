#include "core/deflate.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

bool round_trip(const gcore::ByteVector& input) {
    gcore::DeflateEngine engine;
    const gcore::ByteVector compressed = engine.compress(input);
    const gcore::ByteVector restored = engine.decompress(compressed);
    return restored == input;
}

}  // namespace

int main() {
    const std::string text =
        "GUI Compressor uses LZ77 and Huffman coding for lossless folder compression.";
    const gcore::ByteVector text_bytes(text.begin(), text.end());
    if (!round_trip(text_bytes)) {
        std::cerr << "[FAIL] Deflate text round-trip\n";
        return 1;
    }

    gcore::ByteVector binary(512);
    for (std::size_t i = 0; i < binary.size(); ++i) {
        binary[i] = static_cast<uint8_t>((i * 17) & 0xFF);
    }
    if (!round_trip(binary)) {
        std::cerr << "[FAIL] Deflate binary round-trip\n";
        return 1;
    }

    if (!round_trip({0xAA})) {
        std::cerr << "[FAIL] Deflate single-byte round-trip\n";
        return 1;
    }

    std::cout << "[PASS] Deflate\n";
    return 0;
}

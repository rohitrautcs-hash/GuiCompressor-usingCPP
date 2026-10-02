#include "core/lz77.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

int main() {
    gcore::LZ77 lz77;

    const std::string pattern = "ABCABCABC";
    const gcore::ByteVector input(pattern.begin(), pattern.end());
    const gcore::TokenVector tokens = lz77.compress(input);
    const gcore::ByteVector restored = lz77.decompress(tokens);
    if (restored != input) {
        std::cerr << "[FAIL] LZ77 pattern round-trip\n";
        return 1;
    }

    gcore::ByteVector random_data(1024);
    for (std::size_t i = 0; i < random_data.size(); ++i) {
        random_data[i] = static_cast<uint8_t>(std::rand() & 0xFF);
    }
    const gcore::TokenVector random_tokens = lz77.compress(random_data);
    const gcore::ByteVector random_restored = lz77.decompress(random_tokens);
    if (random_restored != random_data) {
        std::cerr << "[FAIL] LZ77 random round-trip\n";
        return 1;
    }

    const gcore::ByteVector serialized = lz77.serialize(tokens);
    const gcore::TokenVector parsed = lz77.deserialize(serialized);
    const gcore::ByteVector serialized_round = lz77.decompress(parsed);
    if (serialized_round != input) {
        std::cerr << "[FAIL] LZ77 serialize round-trip\n";
        return 1;
    }

    std::cout << "[PASS] LZ77\n";
    return 0;
}

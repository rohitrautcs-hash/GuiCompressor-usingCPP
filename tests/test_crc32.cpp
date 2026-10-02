#include "core/crc32.hpp"

#include <iostream>
#include <string>
#include <vector>

int main() {
    const std::string known = "123456789";
    const std::vector<uint8_t> known_bytes(known.begin(), known.end());
    const uint32_t crc = gcore::CRC32::compute(known_bytes);

    if (crc != 0xCBF43926u) {
        std::cerr << "[FAIL] CRC32 known vector: expected 0xCBF43926 got 0x"
                  << std::hex << crc << '\n';
        return 1;
    }

    const uint32_t empty_crc = gcore::CRC32::compute({});
    if (empty_crc != 0x00000000u) {
        std::cerr << "[FAIL] CRC32 empty: expected 0x00000000 got 0x"
                  << std::hex << empty_crc << '\n';
        return 1;
    }

    std::cout << "[PASS] CRC32\n";
    return 0;
}

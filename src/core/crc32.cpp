#include "crc32.hpp"

namespace gcore {

namespace {

constexpr uint32_t kPolynomial = 0xEDB88320u;

uint32_t build_table_entry(uint32_t index) {
    uint32_t crc = index;
    for (int bit = 0; bit < 8; ++bit) {
        if (crc & 1u) {
            crc = (crc >> 1) ^ kPolynomial;
        } else {
            crc >>= 1;
        }
    }
    return crc;
}

const uint32_t* crc_table() {
    static uint32_t table[256] = {};
    static bool initialized = false;
    if (!initialized) {
        for (uint32_t i = 0; i < 256; ++i) {
            table[i] = build_table_entry(i);
        }
        initialized = true;
    }
    return table;
}

}  // namespace

CRC32::CRC32() : crc_(0xFFFFFFFFu) {}

void CRC32::update(const uint8_t* data, std::size_t length) {
    const uint32_t* table = crc_table();
    for (std::size_t i = 0; i < length; ++i) {
        const uint8_t index = static_cast<uint8_t>((crc_ ^ data[i]) & 0xFFu);
        crc_ = (crc_ >> 8) ^ table[index];
    }
}

void CRC32::update(const std::vector<uint8_t>& data) {
    update(data.data(), data.size());
}

void CRC32::update(const std::string& text) {
    update(reinterpret_cast<const uint8_t*>(text.data()), text.size());
}

uint32_t CRC32::final() const {
    return crc_ ^ 0xFFFFFFFFu;
}

uint32_t CRC32::compute(const std::vector<uint8_t>& data) {
    CRC32 crc;
    crc.update(data);
    return crc.final();
}

}  // namespace gcore

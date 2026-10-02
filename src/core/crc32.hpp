#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace gcore {

/// Computes IEEE 802.3 CRC-32 over arbitrary byte sequences.
class CRC32 {
public:
    CRC32();

    /// Updates the running checksum with more input bytes.
    void update(const uint8_t* data, std::size_t length);

    /// Convenience overload for std::vector payloads.
    void update(const std::vector<uint8_t>& data);

    /// Convenience overload for std::string payloads.
    void update(const std::string& text);

    /// Returns the final 32-bit checksum (not bit-inverted).
    uint32_t final() const;

    /// One-shot CRC over a byte buffer.
    static uint32_t compute(const std::vector<uint8_t>& data);

private:
    uint32_t crc_;
};

}  // namespace gcore

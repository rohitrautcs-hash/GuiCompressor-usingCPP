#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace gcore {

/// Writes bits MSB-first into a byte buffer with bounds-safe reads on the companion reader.
class BitWriter {
public:
    /// Queues a single bit (0 or 1) into the internal byte accumulator.
    void write_bit(bool bit);

    /// Writes the low 8 bits of value, MSB first.
    void write_uint8(uint8_t value);

    /// Writes the low 16 bits of value, MSB first.
    void write_uint16(uint16_t value);

    /// Writes the low 32 bits of value, MSB first.
    void write_uint32(uint32_t value);

    /// Pads the current partial byte with zeros so the next write starts on a byte boundary.
    void flush();

    /// Returns the accumulated bytes; flushes any trailing partial byte first.
    std::vector<uint8_t> finish();

private:
    std::vector<uint8_t> buffer_;
    uint8_t current_byte_ = 0;
    int bit_count_ = 0;
};

/// Reads bits MSB-first from a byte buffer; throws on overrun.
class BitReader {
public:
    explicit BitReader(const std::vector<uint8_t>& data);

    /// Reads the next bit; throws std::runtime_error if the stream is exhausted.
    bool read_bit();

    /// Reads 8 bits as an unsigned byte, MSB first.
    uint8_t read_uint8();

    /// Reads 16 bits as an unsigned integer, MSB first.
    uint16_t read_uint16();

    /// Reads 32 bits as an unsigned integer, MSB first.
    uint32_t read_uint32();

    /// True when all bits have been consumed.
    bool eof() const;

private:
    const std::vector<uint8_t>& data_;
    size_t byte_index_ = 0;
    int bit_index_ = 0;  // 0..7, MSB at 0

    uint8_t read_byte_checked();
};

}  // namespace gcore

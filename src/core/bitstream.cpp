#include "bitstream.hpp"

#include <stdexcept>

namespace gcore {

void BitWriter::write_bit(bool bit) {
    current_byte_ = static_cast<uint8_t>((current_byte_ << 1) | (bit ? 1 : 0));
    ++bit_count_;
    if (bit_count_ == 8) {
        buffer_.push_back(current_byte_);
        current_byte_ = 0;
        bit_count_ = 0;
    }
}

void BitWriter::write_uint8(uint8_t value) {
    for (int i = 7; i >= 0; --i) {
        write_bit(((value >> i) & 1) != 0);
    }
}

void BitWriter::write_uint16(uint16_t value) {
    for (int i = 15; i >= 0; --i) {
        write_bit(((value >> i) & 1) != 0);
    }
}

void BitWriter::write_uint32(uint32_t value) {
    for (int i = 31; i >= 0; --i) {
        write_bit(((value >> i) & 1) != 0);
    }
}

void BitWriter::flush() {
    if (bit_count_ > 0) {
        current_byte_ = static_cast<uint8_t>(current_byte_ << (8 - bit_count_));
        buffer_.push_back(current_byte_);
        current_byte_ = 0;
        bit_count_ = 0;
    }
}

std::vector<uint8_t> BitWriter::finish() {
    flush();
    return buffer_;
}

BitReader::BitReader(const std::vector<uint8_t>& data) : data_(data) {}

bool BitReader::read_bit() {
    if (byte_index_ >= data_.size()) {
        throw std::runtime_error("BitReader: read past end of stream");
    }
    const uint8_t byte = data_[byte_index_];
    const bool bit = ((byte >> (7 - bit_index_)) & 1) != 0;
    ++bit_index_;
    if (bit_index_ == 8) {
        bit_index_ = 0;
        ++byte_index_;
    }
    return bit;
}

uint8_t BitReader::read_byte_checked() {
    if (byte_index_ >= data_.size()) {
        throw std::runtime_error("BitReader: read past end of stream");
    }
    return data_[byte_index_++];
}

uint8_t BitReader::read_uint8() {
    uint8_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value = static_cast<uint8_t>((value << 1) | (read_bit() ? 1 : 0));
    }
    return value;
}

uint16_t BitReader::read_uint16() {
    uint16_t value = 0;
    for (int i = 0; i < 16; ++i) {
        value = static_cast<uint16_t>((value << 1) | (read_bit() ? 1 : 0));
    }
    return value;
}

uint32_t BitReader::read_uint32() {
    uint32_t value = 0;
    for (int i = 0; i < 32; ++i) {
        value = (value << 1) | (read_bit() ? 1u : 0u);
    }
    return value;
}

bool BitReader::eof() const {
    return byte_index_ >= data_.size() && bit_index_ == 0;
}

}  // namespace gcore

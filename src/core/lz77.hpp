#pragma once

#include "types.hpp"

#include <vector>

namespace gcore {

struct MatchResult {
    uint16_t distance = 0;
    uint16_t length = 0;
};

/// LZ77 compressor with a 4096-byte sliding window and 258-byte lookahead.
class LZ77 {
public:
    static constexpr uint16_t kWindowSize = 4096;
    static constexpr uint16_t kLookahead = 258;
    static constexpr uint16_t kMinMatch = 3;

    /// Finds the longest match within the sliding window ending at position pos.
    MatchResult find_match(const ByteVector& data, std::size_t pos) const;

    /// Compresses raw bytes into a sequence of LZ77 tokens.
    TokenVector compress(const ByteVector& data) const;

    /// Reconstructs the original byte stream from tokens.
    ByteVector decompress(const TokenVector& tokens) const;

    /// Serializes tokens to a compact byte representation for Huffman coding.
    ByteVector serialize(const TokenVector& tokens) const;

    /// Parses bytes produced by serialize().
    TokenVector deserialize(const ByteVector& data) const;
};

}  // namespace gcore

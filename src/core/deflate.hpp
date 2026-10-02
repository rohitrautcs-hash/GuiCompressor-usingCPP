#pragma once

#include "types.hpp"

#include <vector>

namespace gcore {

/// Orchestrates LZ77 tokenization followed by Huffman entropy coding.
class DeflateEngine {
public:
    /// Compresses raw bytes using LZ77 + Huffman.
    ByteVector compress(const ByteVector& input) const;

    /// Decompresses bytes produced by compress().
    ByteVector decompress(const ByteVector& input) const;
};

}  // namespace gcore

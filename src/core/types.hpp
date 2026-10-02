#pragma once

#include <cstdint>
#include <vector>

namespace gcore {

/// Represents a single LZ77 compression token.
/// Either a literal byte or a back-reference into the sliding window.
struct LZ77Token {
    /// True when this token encodes a literal byte; false for a back-reference.
    bool is_literal = true;

    /// Literal byte value (valid when is_literal == true).
    uint8_t literal = 0;

    /// Distance back from current position (valid when is_literal == false).
    uint16_t distance = 0;

    /// Length of the repeated sequence (valid when is_literal == false).
    uint16_t length = 0;
};

using ByteVector = std::vector<uint8_t>;
using TokenVector = std::vector<LZ77Token>;

}  // namespace gcore

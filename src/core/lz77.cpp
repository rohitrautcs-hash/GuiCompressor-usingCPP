#include "lz77.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace gcore {

// ---------------------------------------------------------------------------
// Hash-chain constants
// ---------------------------------------------------------------------------
// 15-bit hash table – must be power-of-two.
static constexpr uint32_t kHashSize = 1u << 15;  // 32 768 buckets
static constexpr uint32_t kHashMask = kHashSize - 1;
static constexpr uint32_t kNil      = 0xFFFFFFFFu;
// Maximum chain steps per position (speed vs ratio trade-off).
static constexpr int kMaxChain = 128;

// Fast 3-byte hash.
static inline uint32_t hash3(const uint8_t* p) {
    const uint32_t v = (static_cast<uint32_t>(p[0]) << 16) |
                       (static_cast<uint32_t>(p[1]) <<  8) |
                        static_cast<uint32_t>(p[2]);
    return (v * 2654435761u) & kHashMask;
}

// ---------------------------------------------------------------------------
// find_match() – kept for API compatibility; used by tests.
// The compress() path uses the hash-chain directly for speed.
// ---------------------------------------------------------------------------
MatchResult LZ77::find_match(const ByteVector& data, std::size_t pos) const {
    MatchResult best;
    if (pos + kMinMatch > data.size()) return best;

    const std::size_t window_start = pos > kWindowSize ? pos - kWindowSize : 0;
    const std::size_t max_len = std::min<std::size_t>(kLookahead, data.size() - pos);
    const uint8_t* base = data.data();

    for (std::size_t cand = pos; cand-- > window_start;) {
        std::size_t ml = 0;
        while (ml < max_len && base[cand + ml] == base[pos + ml]) ++ml;
        if (ml >= kMinMatch && ml > best.length) {
            best.length   = static_cast<uint16_t>(ml);
            best.distance = static_cast<uint16_t>(pos - cand);
            if (ml == max_len) break;
        }
    }
    return best;
}

// ---------------------------------------------------------------------------
// compress() – hash-chain O(1) match search
// ---------------------------------------------------------------------------
TokenVector LZ77::compress(const ByteVector& data) const {
    const std::size_t n = data.size();
    TokenVector tokens;
    tokens.reserve(n / 2 + 64);

    if (n == 0) return tokens;

    // head[h] = most-recent absolute position with this hash (or kNil).
    // prev[pos & windowMask] = previous position with the same hash.
    const uint32_t windowMask = kWindowSize - 1;
    std::vector<uint32_t> head(kHashSize, kNil);
    std::vector<uint32_t> prev(kWindowSize, kNil);

    const uint8_t* base = data.data();

    // Insert a position into the hash chain without returning anything.
    auto insert_hash = [&](std::size_t p) {
        if (p + kMinMatch > n) return;
        const uint32_t h = hash3(base + p);
        prev[p & windowMask] = head[h];
        head[h] = static_cast<uint32_t>(p);
    };

    std::size_t pos = 0;

    while (pos < n) {
        // Not enough bytes left for a match.
        if (pos + kMinMatch > n) {
            LZ77Token t;
            t.is_literal = true;
            t.literal    = base[pos++];
            tokens.push_back(t);
            continue;
        }

        const uint32_t h           = hash3(base + pos);
        const uint32_t window_low  = pos > kWindowSize ? static_cast<uint32_t>(pos - kWindowSize) : 0u;
        const std::size_t max_len  = std::min<std::size_t>(kLookahead, n - pos);

        uint16_t best_len  = 0;
        uint16_t best_dist = 0;

        // Walk the hash chain.
        int steps = 0;
        uint32_t cand = head[h];
        while (cand != kNil && cand >= window_low && steps < kMaxChain) {
            const uint8_t* cp = base + cand;
            const uint8_t* pp = base + pos;
            std::size_t ml = 0;
            while (ml < max_len && cp[ml] == pp[ml]) ++ml;
            if (static_cast<uint16_t>(ml) > best_len) {
                best_len  = static_cast<uint16_t>(ml);
                best_dist = static_cast<uint16_t>(pos - cand);
                if (ml == max_len) break;
            }
            cand = prev[cand & windowMask];
            ++steps;
        }

        // Insert current position into chain.
        prev[pos & windowMask] = head[h];
        head[h] = static_cast<uint32_t>(pos);

        if (best_len >= kMinMatch) {
            LZ77Token t;
            t.is_literal = false;
            t.distance   = best_dist;
            t.length     = best_len;
            tokens.push_back(t);
            // Lazy insertion for skipped positions.
            for (uint16_t k = 1; k < best_len; ++k) insert_hash(pos + k);
            pos += best_len;
        } else {
            LZ77Token t;
            t.is_literal = true;
            t.literal    = base[pos];
            tokens.push_back(t);
            ++pos;
        }
    }

    return tokens;
}

// ---------------------------------------------------------------------------
// decompress()
// ---------------------------------------------------------------------------
ByteVector LZ77::decompress(const TokenVector& tokens) const {
    ByteVector output;
    output.reserve(tokens.size() * 2);

    for (const LZ77Token& token : tokens) {
        if (token.is_literal) {
            output.push_back(token.literal);
            continue;
        }
        if (token.distance == 0 || token.distance > output.size()) {
            throw std::runtime_error("LZ77: invalid back-reference distance");
        }
        const std::size_t start = output.size() - token.distance;
        for (uint16_t i = 0; i < token.length; ++i)
            output.push_back(output[start + i]);
    }
    return output;
}

// ---------------------------------------------------------------------------
// serialize() – compact bit-packed format
//
// OLD format cost per token:
//   literal = 2 bytes (0x00 marker + 1 byte value)   ? always expands data!
//   match   = 5 bytes (0x01 marker + 2 dist + 2 len)
//
// NEW bit-packed format:
//   literal : 0  <8-bit value>                    = 9 bits
//   match   : 1  <12-bit dist-1>  <8-bit len-3>  = 21 bits
//
// For typical text this averages ~12 bits/token vs 16–40 bits before.
// Header: 4 bytes big-endian token count so deserialize() knows when to stop.
// ---------------------------------------------------------------------------
ByteVector LZ77::serialize(const TokenVector& tokens) const {
    ByteVector bytes;
    bytes.reserve(tokens.size() * 2 + 4);

    // 4-byte big-endian token count header.
    const uint32_t count = static_cast<uint32_t>(tokens.size());
    bytes.push_back(static_cast<uint8_t>((count >> 24) & 0xFF));
    bytes.push_back(static_cast<uint8_t>((count >> 16) & 0xFF));
    bytes.push_back(static_cast<uint8_t>((count >>  8) & 0xFF));
    bytes.push_back(static_cast<uint8_t>( count        & 0xFF));

    // Inline bit writer.
    uint8_t cur = 0;
    int     fill = 0;

    auto emit_bit = [&](bool b) {
        cur = static_cast<uint8_t>((cur << 1) | (b ? 1u : 0u));
        if (++fill == 8) { bytes.push_back(cur); cur = 0; fill = 0; }
    };

    auto emit_bits = [&](uint32_t value, int nbits) {
        for (int i = nbits - 1; i >= 0; --i)
            emit_bit(((value >> i) & 1u) != 0u);
    };

    for (const LZ77Token& t : tokens) {
        if (t.is_literal) {
            emit_bit(false);
            emit_bits(t.literal, 8);
        } else {
            emit_bit(true);
            emit_bits(static_cast<uint32_t>(t.distance - 1), 12);
            emit_bits(static_cast<uint32_t>(t.length   - kMinMatch), 8);
        }
    }

    if (fill > 0) {
        cur = static_cast<uint8_t>(cur << (8 - fill));
        bytes.push_back(cur);
    }

    return bytes;
}

// ---------------------------------------------------------------------------
// deserialize()
// ---------------------------------------------------------------------------
TokenVector LZ77::deserialize(const ByteVector& data) const {
    if (data.size() < 4)
        throw std::runtime_error("LZ77: truncated header");

    const uint32_t count =
        (static_cast<uint32_t>(data[0]) << 24) |
        (static_cast<uint32_t>(data[1]) << 16) |
        (static_cast<uint32_t>(data[2]) <<  8) |
         static_cast<uint32_t>(data[3]);

    TokenVector tokens;
    tokens.reserve(count);

    // Inline bit reader.
    std::size_t bidx = 4;
    int         boff = 0;  // 0 = MSB of current byte

    auto read_bit = [&]() -> bool {
        if (bidx >= data.size())
            throw std::runtime_error("LZ77: read past end of bit stream");
        const bool b = ((data[bidx] >> (7 - boff)) & 1u) != 0u;
        if (++boff == 8) { boff = 0; ++bidx; }
        return b;
    };

    auto read_bits = [&](int n) -> uint32_t {
        uint32_t v = 0;
        for (int i = 0; i < n; ++i) v = (v << 1) | (read_bit() ? 1u : 0u);
        return v;
    };

    for (uint32_t i = 0; i < count; ++i) {
        LZ77Token t;
        if (!read_bit()) {
            t.is_literal = true;
            t.literal    = static_cast<uint8_t>(read_bits(8));
        } else {
            t.is_literal = false;
            t.distance   = static_cast<uint16_t>(read_bits(12) + 1);
            t.length     = static_cast<uint16_t>(read_bits(8) + kMinMatch);
        }
        tokens.push_back(t);
    }

    return tokens;
}

}  // namespace gcore

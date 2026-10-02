#include "lz77.hpp"

#include <algorithm>
#include <stdexcept>

namespace gcore {

MatchResult LZ77::find_match(const ByteVector& data, std::size_t pos) const {
    MatchResult best;
    if (pos >= data.size()) {
        return best;
    }

    const std::size_t window_start =
        pos > kWindowSize ? pos - kWindowSize : 0;
    const std::size_t max_length =
        std::min<std::size_t>(kLookahead, data.size() - pos);

    for (std::size_t candidate = pos; candidate-- > window_start;) {
        std::size_t match_len = 0;
        while (match_len < max_length &&
               data[candidate + match_len] == data[pos + match_len]) {
            ++match_len;
        }

        if (match_len >= kMinMatch && match_len > best.length) {
            best.length = static_cast<uint16_t>(match_len);
            best.distance = static_cast<uint16_t>(pos - candidate);
            if (match_len == max_length) {
                break;
            }
        }
    }

    return best;
}

TokenVector LZ77::compress(const ByteVector& data) const {
    TokenVector tokens;
    std::size_t pos = 0;

    while (pos < data.size()) {
        const MatchResult match = find_match(data, pos);
        if (match.length >= kMinMatch) {
            LZ77Token token;
            token.is_literal = false;
            token.distance = match.distance;
            token.length = match.length;
            tokens.push_back(token);
            pos += match.length;
        } else {
            LZ77Token token;
            token.is_literal = true;
            token.literal = data[pos];
            tokens.push_back(token);
            ++pos;
        }
    }

    return tokens;
}

ByteVector LZ77::decompress(const TokenVector& tokens) const {
    ByteVector output;

    for (const LZ77Token& token : tokens) {
        if (token.is_literal) {
            output.push_back(token.literal);
            continue;
        }

        if (token.distance == 0 || token.distance > output.size()) {
            throw std::runtime_error("LZ77: invalid back-reference distance");
        }

        const std::size_t start = output.size() - token.distance;
        for (uint16_t i = 0; i < token.length; ++i) {
            output.push_back(output[start + i]);
        }
    }

    return output;
}

ByteVector LZ77::serialize(const TokenVector& tokens) const {
    ByteVector bytes;
    bytes.reserve(tokens.size() * 5);

    for (const LZ77Token& token : tokens) {
        if (token.is_literal) {
            bytes.push_back(0x00);
            bytes.push_back(token.literal);
        } else {
            bytes.push_back(0x01);
            bytes.push_back(static_cast<uint8_t>(token.distance & 0xFF));
            bytes.push_back(static_cast<uint8_t>((token.distance >> 8) & 0xFF));
            bytes.push_back(static_cast<uint8_t>(token.length & 0xFF));
            bytes.push_back(static_cast<uint8_t>((token.length >> 8) & 0xFF));
        }
    }

    return bytes;
}

TokenVector LZ77::deserialize(const ByteVector& data) const {
    TokenVector tokens;
    std::size_t i = 0;

    while (i < data.size()) {
        const uint8_t marker = data[i++];
        if (marker == 0x00) {
            if (i >= data.size()) {
                throw std::runtime_error("LZ77: truncated literal token");
            }
            LZ77Token token;
            token.is_literal = true;
            token.literal = data[i++];
            tokens.push_back(token);
        } else if (marker == 0x01) {
            if (i + 4 > data.size()) {
                throw std::runtime_error("LZ77: truncated match token");
            }
            LZ77Token token;
            token.is_literal = false;
            token.distance = static_cast<uint16_t>(data[i] | (data[i + 1] << 8));
            i += 2;
            token.length = static_cast<uint16_t>(data[i] | (data[i + 1] << 8));
            i += 2;
            tokens.push_back(token);
        } else {
            throw std::runtime_error("LZ77: unknown token marker");
        }
    }

    return tokens;
}

}  // namespace gcore

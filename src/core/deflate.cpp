#include "deflate.hpp"

#include "huffman.hpp"
#include "lz77.hpp"

namespace gcore {

ByteVector DeflateEngine::compress(const ByteVector& input) const {
    const LZ77 lz77;
    const TokenVector tokens = lz77.compress(input);
    const ByteVector serialized = lz77.serialize(tokens);

    const HuffmanCodec codec;
    return codec.compress(serialized);
}

ByteVector DeflateEngine::decompress(const ByteVector& input) const {
    const HuffmanCodec codec;
    const ByteVector serialized = codec.decompress(input);

    const LZ77 lz77;
    const TokenVector tokens = lz77.deserialize(serialized);
    return lz77.decompress(tokens);
}

}  // namespace gcore

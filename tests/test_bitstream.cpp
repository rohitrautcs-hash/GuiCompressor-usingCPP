#include "core/bitstream.hpp"

#include <cassert>
#include <iostream>
#include <vector>

int main() {
    gcore::BitWriter writer;
    writer.write_bit(true);
    writer.write_bit(false);
    writer.write_bit(true);
    writer.write_uint8(0xAB);
    writer.write_uint16(0x1234);

    const std::vector<uint8_t> data = writer.finish();

    gcore::BitReader reader(data);
    assert(reader.read_bit() == true);
    assert(reader.read_bit() == false);
    assert(reader.read_bit() == true);
    assert(reader.read_uint8() == 0xAB);
    assert(reader.read_uint16() == 0x1234);
    // Stream may contain padding bits after the payload; equality checks above are sufficient.

    std::cout << "[PASS] BitStream\n";
    return 0;
}

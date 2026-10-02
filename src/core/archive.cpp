#include "archive.hpp"

#include "crc32.hpp"
#include "deflate.hpp"
#include "filesystem_compat.hpp"

#include <fstream>
#include <stdexcept>

namespace gcore {

constexpr char Archive::kMagic[5];
constexpr uint16_t Archive::kVersion;

namespace {

void write_u16(std::ostream& out, uint16_t value) {
    out.put(static_cast<char>(value & 0xFF));
    out.put(static_cast<char>((value >> 8) & 0xFF));
}

void write_u32(std::ostream& out, uint32_t value) {
    for (int shift = 0; shift < 32; shift += 8) {
        out.put(static_cast<char>((value >> shift) & 0xFF));
    }
}

void write_u64(std::ostream& out, uint64_t value) {
    for (int shift = 0; shift < 64; shift += 8) {
        out.put(static_cast<char>((value >> shift) & 0xFF));
    }
}

uint16_t read_u16(std::istream& in) {
    const int lo = in.get();
    const int hi = in.get();
    if (lo == EOF || hi == EOF) {
        throw std::runtime_error("Archive: unexpected end of file while reading u16");
    }
    return static_cast<uint16_t>(lo | (hi << 8));
}

uint32_t read_u32(std::istream& in) {
    uint32_t value = 0;
    for (int shift = 0; shift < 32; shift += 8) {
        const int byte = in.get();
        if (byte == EOF) {
            throw std::runtime_error("Archive: unexpected end of file while reading u32");
        }
        value |= static_cast<uint32_t>(byte) << shift;
    }
    return value;
}

uint64_t read_u64(std::istream& in) {
    uint64_t value = 0;
    for (int shift = 0; shift < 64; shift += 8) {
        const int byte = in.get();
        if (byte == EOF) {
            throw std::runtime_error("Archive: unexpected end of file while reading u64");
        }
        value |= static_cast<uint64_t>(byte) << shift;
    }
    return value;
}

void write_string(std::ostream& out, const std::string& text) {
    if (text.size() > 0xFFFFu) {
        throw std::runtime_error("Archive: path too long");
    }
    write_u16(out, static_cast<uint16_t>(text.size()));
    out.write(text.data(), static_cast<std::streamsize>(text.size()));
}

std::string read_string(std::istream& in) {
    const uint16_t length = read_u16(in);
    std::string text(length, '\0');
    in.read(&text[0], length);
    if (!in) {
        throw std::runtime_error("Archive: truncated path string");
    }
    return text;
}

}  // namespace

std::vector<uint8_t> Archive::read_file_bytes(const std::string& file_path) const {
    std::ifstream input(file_path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Archive: unable to read file: " + file_path);
    }
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(input), {});
}

void Archive::collect_entries(const std::string& root, const std::string& current,
                              std::vector<ArchiveEntry>& entries) const {
    if (current != root) {
        ArchiveEntry dir_entry;
        dir_entry.relative_path = fs::relative_path(root, current);
        dir_entry.is_directory = true;
        entries.push_back(dir_entry);
    }

    for (const fs::DirectoryEntry& item : fs::directory_entries(current)) {
        if (item.is_directory) {
            collect_entries(root, item.path, entries);
        } else if (item.is_regular_file) {
            const std::vector<uint8_t> raw = read_file_bytes(item.path);

            ArchiveEntry file_entry;
            file_entry.relative_path = fs::relative_path(root, item.path);
            file_entry.original_size = raw.size();
            file_entry.crc32 = CRC32::compute(raw);
            file_entry.is_directory = false;

            DeflateEngine engine;
            const std::vector<uint8_t> compressed = engine.compress(raw);
            if (compressed.size() >= raw.size()) {
                file_entry.method = CompressionMethod::Raw;
                file_entry.payload = raw;
            } else {
                file_entry.method = CompressionMethod::Deflate;
                file_entry.payload = compressed;
            }
            file_entry.compressed_size = file_entry.payload.size();
            entries.push_back(file_entry);
        }
    }
}

void Archive::write_archive_file(const std::string& output_file,
                                 const std::vector<ArchiveEntry>& entries) const {
    const std::string temp_path = output_file + ".tmp";
    {
        std::ofstream out(temp_path, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Archive: unable to create temp file: " + temp_path);
        }

        out.write(kMagic, 4);
        write_u16(out, kVersion);
        out.put(static_cast<char>(0));

        write_u32(out, static_cast<uint32_t>(entries.size()));

        const std::streampos table_start = out.tellp();
        for (const ArchiveEntry& entry : entries) {
            write_string(out, entry.relative_path);
            write_u64(out, entry.original_size);
            write_u64(out, entry.compressed_size);
            write_u32(out, entry.crc32);
            write_u32(out, entry.permissions);
            out.put(entry.is_directory ? 1 : 0);
            out.put(static_cast<char>(entry.method));
            write_u64(out, 0);
        }

        std::vector<uint64_t> offsets;
        offsets.reserve(entries.size());
        for (const ArchiveEntry& entry : entries) {
            offsets.push_back(static_cast<uint64_t>(out.tellp()));
            if (!entry.is_directory && !entry.payload.empty()) {
                out.write(reinterpret_cast<const char*>(entry.payload.data()),
                          static_cast<std::streamsize>(entry.payload.size()));
            }
        }

        out.seekp(table_start);
        for (std::size_t i = 0; i < entries.size(); ++i) {
            write_string(out, entries[i].relative_path);
            write_u64(out, entries[i].original_size);
            write_u64(out, entries[i].compressed_size);
            write_u32(out, entries[i].crc32);
            write_u32(out, entries[i].permissions);
            out.put(entries[i].is_directory ? 1 : 0);
            out.put(static_cast<char>(entries[i].method));
            write_u64(out, offsets[i]);
        }
    }

    fs::rename_file(temp_path, output_file);
}

std::vector<ArchiveEntry> Archive::read_archive_file(const std::string& archive_file,
                                                     bool load_payload) const {
    std::ifstream in(archive_file, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Archive: unable to open archive: " + archive_file);
    }

    char magic[4] = {};
    in.read(magic, 4);
    if (std::string(magic, 4) != kMagic) {
        throw std::runtime_error("Archive: invalid magic header");
    }

    const uint16_t version = read_u16(in);
    if (version != kVersion) {
        throw std::runtime_error("Archive: unsupported version");
    }

    in.get();
    const uint32_t entry_count = read_u32(in);

    std::vector<ArchiveEntry> entries;
    entries.reserve(entry_count);

    for (uint32_t i = 0; i < entry_count; ++i) {
        ArchiveEntry entry;
        entry.relative_path = read_string(in);
        entry.original_size = read_u64(in);
        entry.compressed_size = read_u64(in);
        entry.crc32 = read_u32(in);
        entry.permissions = read_u32(in);
        entry.is_directory = in.get() != 0;
        entry.method = static_cast<CompressionMethod>(in.get());
        entry.data_offset = read_u64(in);
        entries.push_back(entry);
    }

    if (load_payload) {
        DeflateEngine engine;
        for (ArchiveEntry& entry : entries) {
            if (entry.is_directory || entry.compressed_size == 0) {
                continue;
            }
            in.seekg(static_cast<std::streamoff>(entry.data_offset));
            entry.payload.resize(static_cast<std::size_t>(entry.compressed_size));
            in.read(reinterpret_cast<char*>(entry.payload.data()),
                    static_cast<std::streamsize>(entry.compressed_size));
            if (!in) {
                throw std::runtime_error("Archive: truncated payload for " + entry.relative_path);
            }

            std::vector<uint8_t> decompressed;
            if (entry.method == CompressionMethod::Raw) {
                decompressed = entry.payload;
            } else {
                decompressed = engine.decompress(entry.payload);
            }

            if (decompressed.size() != entry.original_size) {
                throw std::runtime_error("Archive: size mismatch for " + entry.relative_path);
            }
            if (CRC32::compute(decompressed) != entry.crc32) {
                throw std::runtime_error("Archive: CRC mismatch for " + entry.relative_path);
            }
        }
    }

    return entries;
}

void Archive::compress_folder(const std::string& source_folder,
                              const std::string& output_file) const {
    if (!fs::exists(source_folder) || !fs::is_directory(source_folder)) {
        throw std::runtime_error("Archive: source folder does not exist");
    }

    std::vector<ArchiveEntry> entries;
    collect_entries(source_folder, source_folder, entries);
    write_archive_file(output_file, entries);
}

void Archive::extract_archive(const std::string& archive_file,
                              const std::string& destination_folder) const {
    fs::create_directories(destination_folder);
    const std::vector<ArchiveEntry> entries = read_archive_file(archive_file, true);
    DeflateEngine engine;

    for (const ArchiveEntry& entry : entries) {
        const std::string out_path = fs::join(destination_folder, entry.relative_path);

        if (entry.is_directory) {
            fs::create_directories(out_path);
            continue;
        }

        const std::size_t slash = out_path.find_last_of("/\\");
        if (slash != std::string::npos) {
            fs::create_directories(out_path.substr(0, slash));
        }

        std::vector<uint8_t> decompressed;
        if (entry.method == CompressionMethod::Raw) {
            decompressed = entry.payload;
        } else {
            decompressed = engine.decompress(entry.payload);
        }

        if (decompressed.size() != entry.original_size) {
            throw std::runtime_error("Archive: extracted size mismatch for " + entry.relative_path);
        }
        if (CRC32::compute(decompressed) != entry.crc32) {
            throw std::runtime_error("Archive: extracted CRC mismatch for " + entry.relative_path);
        }

        std::ofstream out(out_path, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Archive: unable to write " + out_path);
        }
        out.write(reinterpret_cast<const char*>(decompressed.data()),
                  static_cast<std::streamsize>(decompressed.size()));
    }
}

std::vector<ArchiveEntry> Archive::list_entries(const std::string& archive_file) const {
    return read_archive_file(archive_file, false);
}

}  // namespace gcore

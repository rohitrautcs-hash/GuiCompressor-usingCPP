#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace gcore {

/// Compression method stored per archive entry.
enum class CompressionMethod : uint8_t {
    Deflate = 0,
    Raw = 1,
};

/// Metadata for one file or directory inside a .gcf archive.
struct ArchiveEntry {
    std::string relative_path;
    uint64_t original_size = 0;
    uint64_t compressed_size = 0;
    uint32_t crc32 = 0;
    uint32_t permissions = 0;
    bool is_directory = false;
    CompressionMethod method = CompressionMethod::Deflate;
    uint64_t data_offset = 0;
    std::vector<uint8_t> payload;
};

/// Reads and writes GUI Compression Format v1 (.gcf) folder archives.
class Archive {
public:
    static constexpr char kMagic[5] = "GCF1";
    static constexpr uint16_t kVersion = 1;

    /// Recursively compresses a folder into a .gcf file (atomic write via .tmp).
    void compress_folder(const std::string& source_folder, const std::string& output_file) const;

    /// Extracts a .gcf archive into destination_folder with CRC verification.
    void extract_archive(const std::string& archive_file,
                         const std::string& destination_folder) const;

    /// Returns entry metadata without extracting payload bytes.
    std::vector<ArchiveEntry> list_entries(const std::string& archive_file) const;

private:
    void collect_entries(const std::string& root, const std::string& current,
                         std::vector<ArchiveEntry>& entries) const;

    std::vector<uint8_t> read_file_bytes(const std::string& file_path) const;
    void write_archive_file(const std::string& output_file,
                            const std::vector<ArchiveEntry>& entries) const;
    std::vector<ArchiveEntry> read_archive_file(const std::string& archive_file,
                                                bool load_payload) const;
};

}  // namespace gcore

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace gcore {
namespace fs {

struct DirectoryEntry {
    std::string path;
    bool is_directory = false;
    bool is_regular_file = false;
};

/// Returns true when path exists on disk.
bool exists(const std::string& path);

/// Returns true when path points to a directory.
bool is_directory(const std::string& path);

/// Creates a directory and any missing parents.
void create_directories(const std::string& path);

/// Removes a file or empty directory; ignores missing paths.
void remove(const std::string& path);

/// Recursively deletes a directory tree.
void remove_all(const std::string& path);

/// Atomically renames a file (used for .tmp → final archive).
void rename_file(const std::string& from, const std::string& to);

/// Lists one directory level (non-recursive).
std::vector<DirectoryEntry> directory_entries(const std::string& path);

/// Computes a forward-slash relative path from root to target.
std::string relative_path(const std::string& root, const std::string& target);

/// Joins two path segments with a single separator.
std::string join(const std::string& left, const std::string& right);

}  // namespace fs
}  // namespace gcore

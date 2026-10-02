#include "filesystem_compat.hpp"

#include <cerrno>
#include <cstdio>
#include <stdexcept>
#include <sys/stat.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

namespace gcore {
namespace fs {

namespace {

bool stat_is_dir(const std::string& path) {
    struct stat info {};
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
#ifdef _WIN32
    return (info.st_mode & _S_IFDIR) != 0;
#else
    return S_ISDIR(info.st_mode);
#endif
}

bool stat_is_file(const std::string& path) {
    struct stat info {};
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
#ifdef _WIN32
    return (info.st_mode & _S_IFREG) != 0;
#else
    return S_ISREG(info.st_mode);
#endif
}

}  // namespace

bool exists(const std::string& path) {
    struct stat info {};
    return stat(path.c_str(), &info) == 0;
}

bool is_directory(const std::string& path) {
    return exists(path) && stat_is_dir(path);
}

void create_directories(const std::string& path) {
    if (path.empty() || exists(path)) {
        return;
    }

    const std::size_t slash = path.find_last_of("/\\");
    if (slash != std::string::npos) {
        create_directories(path.substr(0, slash));
    }

#ifdef _WIN32
    if (CreateDirectoryA(path.c_str(), nullptr) == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
        throw std::runtime_error("fs: unable to create directory: " + path);
    }
#else
    if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
        throw std::runtime_error("fs: unable to create directory: " + path);
    }
#endif
}

void remove(const std::string& path) {
#ifdef _WIN32
    if (is_directory(path)) {
        RemoveDirectoryA(path.c_str());
    } else {
        DeleteFileA(path.c_str());
    }
#else
    std::remove(path.c_str());
#endif
}

void remove_all(const std::string& path) {
    if (!exists(path)) {
        return;
    }
    if (is_directory(path)) {
        for (const DirectoryEntry& entry : directory_entries(path)) {
            remove_all(entry.path);
        }
        remove(path);
    } else {
        remove(path);
    }
}

void rename_file(const std::string& from, const std::string& to) {
#ifdef _WIN32
    if (MoveFileExA(from.c_str(), to.c_str(), MOVEFILE_REPLACE_EXISTING) == 0) {
        throw std::runtime_error("fs: rename failed");
    }
#else
    if (std::rename(from.c_str(), to.c_str()) != 0) {
        throw std::runtime_error("fs: rename failed");
    }
#endif
}

std::vector<DirectoryEntry> directory_entries(const std::string& path) {
    std::vector<DirectoryEntry> entries;

#ifdef _WIN32
    const std::string pattern = join(path, "*");
    WIN32_FIND_DATAA data {};
    HANDLE handle = FindFirstFileA(pattern.c_str(), &data);
    if (handle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("fs: unable to read directory: " + path);
    }

    do {
        const std::string name = data.cFileName;
        if (name == "." || name == "..") {
            continue;
        }
        DirectoryEntry entry;
        entry.path = join(path, name);
        entry.is_directory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        entry.is_regular_file = !entry.is_directory;
        entries.push_back(entry);
    } while (FindNextFileA(handle, &data));

    FindClose(handle);
#else
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        throw std::runtime_error("fs: unable to read directory: " + path);
    }

    while (dirent* item = readdir(dir)) {
        const std::string name = item->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        DirectoryEntry entry;
        entry.path = join(path, name);
        entry.is_directory = stat_is_dir(entry.path);
        entry.is_regular_file = stat_is_file(entry.path);
        entries.push_back(entry);
    }
    closedir(dir);
#endif

    return entries;
}

std::string relative_path(const std::string& root, const std::string& target) {
    if (root.empty()) {
        return target;
    }

    std::string normalized_root = root;
    std::string normalized_target = target;
    for (char& ch : normalized_root) {
        if (ch == '\\') {
            ch = '/';
        }
    }
    for (char& ch : normalized_target) {
        if (ch == '\\') {
            ch = '/';
        }
    }

    // Strip trailing slash if present for initial comparison
    if (normalized_root.length() > 1 && normalized_root.back() == '/') {
        normalized_root.pop_back();
    }
    if (normalized_target.length() > 1 && normalized_target.back() == '/') {
        normalized_target.pop_back();
    }

    if (normalized_target == normalized_root) {
        return "";
    }

    if (!normalized_root.empty() && normalized_root.back() != '/') {
        normalized_root.push_back('/');
    }

#ifdef _WIN32
    std::string lower_root = normalized_root;
    std::string lower_target = normalized_target;
    for (char& c : lower_root) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    for (char& c : lower_target) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (lower_target.rfind(lower_root, 0) != 0) {
        throw std::runtime_error("fs: target is outside root");
    }
#else
    if (normalized_target.rfind(normalized_root, 0) != 0) {
        throw std::runtime_error("fs: target is outside root");
    }
#endif

    return normalized_target.substr(normalized_root.size());
}

std::string join(const std::string& left, const std::string& right) {
    if (left.empty()) {
        return right;
    }
    if (right.empty()) {
        return left;
    }
    if (left.back() == '/' || left.back() == '\\') {
        return left + right;
    }
#ifdef _WIN32
    return left + "\\" + right;
#else
    return left + "/" + right;
#endif
}

}  // namespace fs
}  // namespace gcore

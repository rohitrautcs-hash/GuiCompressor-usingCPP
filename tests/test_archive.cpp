#include "core/archive.hpp"
#include "core/filesystem_compat.hpp"

#include <fstream>
#include <iostream>
#include <string>

int main() {
    const std::string source = "test_data/source";
    const std::string output = "test_data/test.gcf";
    const std::string extracted = "test_data/out";

    gcore::fs::remove_all(source);
    gcore::fs::remove_all(extracted);
    gcore::fs::remove(output);

    gcore::fs::create_directories(source + "/docs");
    {
        std::ofstream(source + "/readme.txt") << "Hello GCF archive!";
        std::ofstream(source + "/docs/notes.txt") << "Nested file content.";
        std::ofstream bin(source + "/binary.bin", std::ios::binary | std::ios::trunc);
        for (int i = 0; i < 256; ++i) {
            const char byte = static_cast<char>(i);
            bin.write(&byte, 1);
        }
    }

    gcore::Archive archive;
    archive.compress_folder(source, output);

    const auto entries = archive.list_entries(output);
    if (entries.size() < 4) {
        std::cerr << "[FAIL] Archive entry count expected >= 4 got " << entries.size() << '\n';
        return 1;
    }

    archive.extract_archive(output, extracted);

    auto verify_file = [&](const std::string& relative) {
        std::ifstream original(gcore::fs::join(source, relative), std::ios::binary);
        std::ifstream copy(gcore::fs::join(extracted, relative), std::ios::binary);
        const std::string a((std::istreambuf_iterator<char>(original)), {});
        const std::string b((std::istreambuf_iterator<char>(copy)), {});
        if (a != b) {
            std::cerr << "[FAIL] Archive content mismatch for " << relative << '\n';
            return false;
        }
        return true;
    };

    if (!verify_file("readme.txt") || !verify_file("docs/notes.txt") ||
        !verify_file("binary.bin")) {
        return 1;
    }

    std::cout << "[PASS] Archive\n";
    return 0;
}

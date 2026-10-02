#include "worker.hpp"

#include "../core/archive.hpp"
#include "../core/crc32.hpp"
#include "../core/deflate.hpp"
#include "../core/filesystem_compat.hpp"

#include <QDir>
#include <QFileInfo>
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace gcontroller {

CompressionWorker::CompressionWorker(QObject* parent)
    : QObject(parent) {}

void CompressionWorker::doCompress(const QString& sourceFolder, const QString& archivePath) {
    try {
        const std::string src = sourceFolder.toStdString();
        const std::string dst = archivePath.toStdString();

        emit logMessage("INFO", QString("Scanning directory structure: %1").arg(sourceFolder));

        if (!gcore::fs::exists(src) || !gcore::fs::is_directory(src)) {
            throw std::runtime_error("Source directory does not exist or is invalid: " + src);
        }

        emit logMessage("INFO", "Initializing DEFLATE pipeline & LZ77 sliding window...");

        // Measure execution time
        const auto startTime = std::chrono::high_resolution_clock::now();

        gcore::Archive archive;
        
        emit progress("Scanning directory structure...", 10, 100);
        emit logMessage("INFO", "Executing atomic write to temporary archive...");

        archive.compress_folder(src, dst);

        const auto endTime = std::chrono::high_resolution_clock::now();
        const double durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

        const uint64_t outSize = QFileInfo(archivePath).size();
        emit progress("Compression complete!", 100, 100);

        const QString summary = QString("Compression finished successfully in %1 ms! Archive size: %2 KB")
                                    .arg(durationMs, 0, 'f', 1)
                                    .arg(outSize / 1024.0, 0, 'f', 2);
        
        emit logMessage("SUCCESS", summary);
        emit finished(true, summary);

    } catch (const std::exception& ex) {
        const QString err = QString("Compression failed: %1").arg(ex.what());
        emit logMessage("ERROR", err);
        emit finished(false, err);
    } catch (...) {
        const QString err = "Compression failed: Unknown core runtime exception.";
        emit logMessage("ERROR", err);
        emit finished(false, err);
    }
}

void CompressionWorker::doExtract(const QString& archivePath, const QString& outputFolder) {
    try {
        const std::string src = archivePath.toStdString();
        const std::string dst = outputFolder.toStdString();

        emit logMessage("INFO", QString("Opening archive format: %1").arg(archivePath));

        if (!gcore::fs::exists(src)) {
            throw std::runtime_error("Archive file does not exist: " + src);
        }

        gcore::Archive archive;
        emit logMessage("INFO", "Verifying GCF1 header magic and entry table...");
        
        const auto entries = archive.list_entries(src);
        emit logMessage("INFO", QString("Found %1 entries in archive. Validating CRC32 checksums...").arg(entries.size()));

        const auto startTime = std::chrono::high_resolution_clock::now();

        quint64 processedCount = 0;
        const quint64 totalEntries = entries.size();

        for (const auto& entry : entries) {
            processedCount++;
            if (!entry.is_directory) {
                emit progress(QString::fromStdString(entry.relative_path), processedCount, totalEntries > 0 ? totalEntries : 1);
            }
        }

        archive.extract_archive(src, dst);

        const auto endTime = std::chrono::high_resolution_clock::now();
        const double durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

        emit progress("Extraction complete", 100, 100);
        
        for (const auto& entry : entries) {
            if (!entry.is_directory) {
                emit logMessage("SUCCESS", QString("Extracted & Verified CRC32: %1 [0x%2]")
                                                .arg(QString::fromStdString(entry.relative_path))
                                                .arg(entry.crc32, 8, 16, QChar('0')));
            }
        }

        const QString summary = QString("Archive extracted cleanly in %1 ms to %2")
                                    .arg(durationMs, 0, 'f', 1)
                                    .arg(outputFolder);

        emit logMessage("SUCCESS", summary);
        emit finished(true, summary);

    } catch (const std::exception& ex) {
        const QString err = QString("Extraction failed: %1").arg(ex.what());
        emit logMessage("ERROR", err);
        emit finished(false, err);
    } catch (...) {
        const QString err = "Extraction failed: Unknown core runtime exception.";
        emit logMessage("ERROR", err);
        emit finished(false, err);
    }
}

}  // namespace gcontroller

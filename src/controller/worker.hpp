#pragma once

#include <QObject>
#include <QString>
#include <cstdint>

namespace gcontroller {

/**
 * @brief Qt Worker thread controller bridge for background compression & extraction.
 * 
 * Runs heavy C++ core archive tasks inside a dedicated QThread to ensure the UI
 * remains smooth, highly responsive, and unblocked. Emits real-time progress,
 * styled log notifications, and final status callbacks.
 */
class CompressionWorker : public QObject {
    Q_OBJECT

public:
    explicit CompressionWorker(QObject* parent = nullptr);
    ~CompressionWorker() override = default;

public slots:
    /**
     * @brief Performs lossless folder compression to a .gcf archive.
     * @param sourceFolder Path to target directory.
     * @param archivePath Path to output .gcf archive file.
     */
    void doCompress(const QString& sourceFolder, const QString& archivePath);

    /**
     * @brief Performs extraction of a .gcf archive into a target folder.
     * @param archivePath Path to .gcf archive file.
     * @param outputFolder Path to output destination directory.
     */
    void doExtract(const QString& archivePath, const QString& outputFolder);

signals:
    /**
     * @brief Real-time operation progress report.
     * @param currentFile Relative path of item being processed.
     * @param processedBytes Cumulative bytes processed so far.
     * @param totalBytes Total dataset size in bytes.
     */
    void progress(const QString& currentFile, quint64 processedBytes, quint64 totalBytes);

    /**
     * @brief Console log notification signal.
     * @param level Severity level ("INFO", "SUCCESS", "WARNING", "ERROR").
     * @param message Text contents to append to log.
     */
    void logMessage(const QString& level, const QString& message);

    /**
     * @brief Operation complete callback signal.
     * @param success True if finished without error.
     * @param message Outcome summary or exception details.
     */
    void finished(bool success, const QString& message);
};

}  // namespace gcontroller

#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QTableWidget>
#include <QLabel>
#include <QThread>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QClipboard>
#include <QKeyEvent>

#include "logpanel.hpp"
#include "../controller/worker.hpp"

namespace ggui {

/**
 * @brief Extract Page for multi-format archive inspection and extraction.
 *
 * Features:
 *  - Drag-and-drop / paste archive file selection
 *  - Smart auto-fill of destination path (parent dir of archive)
 *  - Toggleable "Advanced Details" metadata table (CRC-32, Method, sizes)
 *  - Thread-offloaded DEFLATE decompression with CRC32 verification
 */
class ExtractPage : public QWidget {
    Q_OBJECT

public:
    explicit ExtractPage(QWidget* parent = nullptr);
    ~ExtractPage() override;

protected:
    // Drag-and-drop support
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    // Clipboard paste support (Ctrl+V)
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onBrowseArchive();
    void onBrowseDestination();
    void onStartExtract();
    void onWorkerProgress(const QString& currentFile, quint64 processed, quint64 total);
    void onWorkerFinished(bool success, const QString& message);
    void inspectArchive(const QString& archivePath);
    void onArchivePathChanged(const QString& path);
    void onToggleDetails();

private:
    void setupUi();
    void loadArchiveFile(const QString& path);

    // Drop zone / archive input
    QWidget*     m_dropZone{nullptr};
    QLineEdit*   m_archiveInput{nullptr};
    QPushButton* m_browseArchiveBtn{nullptr};

    // Destination
    QLineEdit*   m_destInput{nullptr};
    QPushButton* m_browseDestBtn{nullptr};

    // Details panel (collapsible)
    QWidget*     m_detailsPanel{nullptr};
    QPushButton* m_detailsToggleBtn{nullptr};
    QTableWidget* m_previewTable{nullptr};
    bool         m_detailsVisible{false};

    // Status / action
    QPushButton* m_startBtn{nullptr};
    QProgressBar* m_progressBar{nullptr};
    QLabel*      m_statusLabel{nullptr};

    LogPanel*    m_logPanel{nullptr};

    QThread*                        m_workerThread{nullptr};
    gcontroller::CompressionWorker* m_worker{nullptr};
};

}  // namespace ggui

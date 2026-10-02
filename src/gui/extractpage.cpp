#include "extractpage.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QFileInfo>
#include <QDir>
#include <QScrollArea>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QUrl>
#include <QFrame>

#include "../core/archive.hpp"

namespace ggui {

// ─────────────────────────────────────────────────────────────────────────────
//  Construction / Destruction
// ─────────────────────────────────────────────────────────────────────────────

ExtractPage::ExtractPage(QWidget* parent)
    : QWidget(parent) {
    setAcceptDrops(true);
    setupUi();
}

ExtractPage::~ExtractPage() {
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI Construction
// ─────────────────────────────────────────────────────────────────────────────

void ExtractPage::setupUi() {
    // Root layout (fills the tab)
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    auto* content = new QWidget(scrollArea);
    content->setStyleSheet("background: transparent;");

    auto* mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(28, 24, 28, 24);
    mainLayout->setSpacing(18);

    // Title
    auto* titleLabel = new QLabel("ARCHIVE EXTRACTION", content);
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: bold; color: #00f0ff; letter-spacing: 1.5px;");

    auto* descLabel = new QLabel(
        "Drop or browse any archive, then extract it to a chosen folder.", content);
    descLabel->setStyleSheet("color: #888899; font-size: 13px;");

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(descLabel);

    // Input Card
    auto* cardWidget = new QWidget(content);
    cardWidget->setStyleSheet(
        "background-color: #14141e; border: 1px solid #1a1a2e; border-radius: 10px;");
    auto* cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setContentsMargins(20, 20, 20, 20);
    cardLayout->setSpacing(14);

    // Drop-zone / archive input
    auto* archiveLabel = new QLabel("Source Compressed Archive:", cardWidget);
    archiveLabel->setStyleSheet("color: #cccccc; font-weight: 600; font-size: 13px;");

    // Wrapper widget styled as a drop-zone
    m_dropZone = new QWidget(cardWidget);
    m_dropZone->setObjectName("dropZone");
    m_dropZone->setStyleSheet(
        "QWidget#dropZone {"
        "    background-color: #0e0e1a;"
        "    border: 2px dashed #2a2a4a;"
        "    border-radius: 8px;"
        "}"
        "QWidget#dropZone:hover {"
        "    border-color: #00f0ff;"
        "}"
    );
    m_dropZone->setMinimumHeight(70);

    auto* dropLayout = new QVBoxLayout(m_dropZone);
    dropLayout->setContentsMargins(14, 10, 14, 10);
    dropLayout->setSpacing(6);

    auto* dropHint = new QLabel(
        "  Drag & drop an archive here  or paste a path (Ctrl+V)", m_dropZone);
    dropHint->setAlignment(Qt::AlignCenter);
    dropHint->setStyleSheet(
        "color: #555577; font-size: 12px; border: none; background: transparent;");

    auto* archiveRow = new QHBoxLayout();
    archiveRow->setSpacing(8);
    m_archiveInput = new QLineEdit(m_dropZone);
    m_archiveInput->setPlaceholderText("Path to archive (.gcf, .zip, .rar, .7z, ...)");
    m_archiveInput->setFixedHeight(36);
    m_archiveInput->setStyleSheet(
        "QLineEdit {"
        "    background-color: #1a1a24;"
        "    border: 1px solid #2a2a3a;"
        "    border-radius: 6px;"
        "    color: #ffffff;"
        "    padding: 0 12px;"
        "}"
        "QLineEdit:focus { border-color: #00f0ff; }"
    );

    m_browseArchiveBtn = new QPushButton("Browse...", m_dropZone);
    m_browseArchiveBtn->setFixedSize(100, 36);
    m_browseArchiveBtn->setCursor(Qt::PointingHandCursor);
    m_browseArchiveBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #1f1f2e;"
        "    color: #00f0ff;"
        "    border: 1px solid #00f0ff;"
        "    border-radius: 6px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #00f0ff;"
        "    color: #000000;"
        "}"
    );
    connect(m_browseArchiveBtn, &QPushButton::clicked, this, &ExtractPage::onBrowseArchive);

    archiveRow->addWidget(m_archiveInput);
    archiveRow->addWidget(m_browseArchiveBtn);

    dropLayout->addWidget(dropHint);
    dropLayout->addLayout(archiveRow);

    // Auto-fill destination when the archive path changes
    connect(m_archiveInput, &QLineEdit::textChanged, this, &ExtractPage::onArchivePathChanged);

    // Destination input
    auto* destLabel = new QLabel("Destination Folder:", cardWidget);
    destLabel->setStyleSheet("color: #cccccc; font-weight: 600; font-size: 13px;");

    auto* destRow = new QHBoxLayout();
    destRow->setSpacing(8);
    m_destInput = new QLineEdit(cardWidget);
    m_destInput->setPlaceholderText("Auto-filled from archive location (optional)");
    m_destInput->setFixedHeight(36);
    m_destInput->setStyleSheet(
        "QLineEdit {"
        "    background-color: #1a1a24;"
        "    border: 1px solid #2a2a3a;"
        "    border-radius: 6px;"
        "    color: #ffffff;"
        "    padding: 0 12px;"
        "}"
        "QLineEdit:focus { border-color: #00f0ff; }"
    );

    m_browseDestBtn = new QPushButton("Browse...", cardWidget);
    m_browseDestBtn->setFixedSize(100, 36);
    m_browseDestBtn->setCursor(Qt::PointingHandCursor);
    m_browseDestBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #1f1f2e;"
        "    color: #00f0ff;"
        "    border: 1px solid #00f0ff;"
        "    border-radius: 6px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #00f0ff;"
        "    color: #000000;"
        "}"
    );
    connect(m_browseDestBtn, &QPushButton::clicked, this, &ExtractPage::onBrowseDestination);

    destRow->addWidget(m_destInput);
    destRow->addWidget(m_browseDestBtn);

    cardLayout->addWidget(archiveLabel);
    cardLayout->addWidget(m_dropZone);
    cardLayout->addWidget(destLabel);
    cardLayout->addLayout(destRow);

    mainLayout->addWidget(cardWidget);

    // Advanced / Details toggle header
    auto* detailsHeaderRow = new QHBoxLayout();
    auto* detailsHeaderLabel = new QLabel("ARCHIVE CONTENTS", content);
    detailsHeaderLabel->setStyleSheet(
        "font-size: 12px; font-weight: bold; color: #555577; letter-spacing: 1px;");

    m_detailsToggleBtn = new QPushButton("Show Details", content);
    m_detailsToggleBtn->setCursor(Qt::PointingHandCursor);
    m_detailsToggleBtn->setFixedHeight(26);
    m_detailsToggleBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: #00f0ff;"
        "    border: 1px solid #1a1a3a;"
        "    border-radius: 5px;"
        "    padding: 0 10px;"
        "    font-size: 11px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover { border-color: #00f0ff; }"
    );
    connect(m_detailsToggleBtn, &QPushButton::clicked, this, &ExtractPage::onToggleDetails);

    detailsHeaderRow->addWidget(detailsHeaderLabel);
    detailsHeaderRow->addStretch();
    detailsHeaderRow->addWidget(m_detailsToggleBtn);
    mainLayout->addLayout(detailsHeaderRow);

    // Collapsible details panel (hidden by default)
    m_detailsPanel = new QWidget(content);
    m_detailsPanel->setVisible(false);

    auto* detailsPanelLayout = new QVBoxLayout(m_detailsPanel);
    detailsPanelLayout->setContentsMargins(0, 0, 0, 0);
    detailsPanelLayout->setSpacing(0);

    m_previewTable = new QTableWidget(0, 5, m_detailsPanel);
    m_previewTable->setHorizontalHeaderLabels(
        {"File Path", "Original Size", "Compressed", "Method", "CRC-32"});
    m_previewTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_previewTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_previewTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_previewTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_previewTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_previewTable->verticalHeader()->setVisible(false);
    m_previewTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_previewTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_previewTable->setAlternatingRowColors(true);
    m_previewTable->setMinimumHeight(160);
    m_previewTable->setMaximumHeight(260);
    m_previewTable->setStyleSheet(
        "QTableWidget {"
        "    background-color: #10101a;"
        "    alternate-background-color: #141424;"
        "    color: #dddddd;"
        "    gridline-color: #1a1a2e;"
        "    border: 1px solid #1a1a2e;"
        "    border-radius: 8px;"
        "    font-size: 12px;"
        "}"
        "QHeaderView::section {"
        "    background-color: #161626;"
        "    color: #00f0ff;"
        "    font-weight: bold;"
        "    border: none;"
        "    border-bottom: 1px solid #00f0ff;"
        "    padding: 8px 10px;"
        "}"
        "QTableWidget::item { padding: 6px 10px; }"
        "QTableWidget::item:selected {"
        "    background-color: #00f0ff;"
        "    color: #000000;"
        "    font-weight: bold;"
        "}"
    );

    detailsPanelLayout->addWidget(m_previewTable);
    mainLayout->addWidget(m_detailsPanel);

    // Status & Progress
    m_statusLabel = new QLabel("Select or drop an archive to get started.", content);
    m_statusLabel->setStyleSheet("color: #aaaaaa; font-size: 12px; font-weight: 500;");

    m_progressBar = new QProgressBar(content);
    m_progressBar->setFixedHeight(14);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "    background-color: #1a1a24;"
        "    border: 1px solid #2a2a3a;"
        "    border-radius: 7px;"
        "}"
        "QProgressBar::chunk {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f0ff, stop:1 #39ff14);"
        "    border-radius: 6px;"
        "}"
    );

    // Primary Action Button
    m_startBtn = new QPushButton("  UNPACK ARCHIVE", content);
    m_startBtn->setFixedHeight(44);
    m_startBtn->setCursor(Qt::PointingHandCursor);
    m_startBtn->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f0ff, stop:1 #00a8ff);"
        "    color: #000000;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    border: none;"
        "    border-radius: 8px;"
        "    letter-spacing: 1px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #39ff14, stop:1 #00f0ff);"
        "}"
        "QPushButton:disabled {"
        "    background-color: #222230;"
        "    color: #555566;"
        "}"
    );
    connect(m_startBtn, &QPushButton::clicked, this, &ExtractPage::onStartExtract);

    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addWidget(m_startBtn);

    // Log Panel
    m_logPanel = new LogPanel(content);
    m_logPanel->setMinimumHeight(180);
    mainLayout->addWidget(m_logPanel, 1);

    scrollArea->setWidget(content);
    rootLayout->addWidget(scrollArea);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Drag-and-Drop
// ─────────────────────────────────────────────────────────────────────────────

void ExtractPage::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        const auto urls = event->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().isLocalFile()) {
            // Highlight drop zone
            m_dropZone->setStyleSheet(
                "QWidget#dropZone {"
                "    background-color: #00f0ff18;"
                "    border: 2px dashed #00f0ff;"
                "    border-radius: 8px;"
                "}"
            );
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void ExtractPage::dropEvent(QDropEvent* event) {
    // Restore drop-zone style
    m_dropZone->setStyleSheet(
        "QWidget#dropZone {"
        "    background-color: #0e0e1a;"
        "    border: 2px dashed #2a2a4a;"
        "    border-radius: 8px;"
        "}"
        "QWidget#dropZone:hover {"
        "    border-color: #00f0ff;"
        "}"
    );

    if (event->mimeData()->hasUrls()) {
        const auto urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            const QString path = urls.first().toLocalFile();
            loadArchiveFile(path);
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Clipboard Paste (Ctrl+V)
// ─────────────────────────────────────────────────────────────────────────────

void ExtractPage::keyPressEvent(QKeyEvent* event) {
    if (event->matches(QKeySequence::Paste)) {
        const QClipboard* cb = QApplication::clipboard();
        const QMimeData* md = cb->mimeData();
        if (md->hasUrls()) {
            const auto urls = md->urls();
            if (!urls.isEmpty() && urls.first().isLocalFile()) {
                loadArchiveFile(urls.first().toLocalFile());
                return;
            }
        }
        // Try plain text as a file path
        const QString text = cb->text().trimmed();
        if (!text.isEmpty()) {
            loadArchiveFile(text);
            return;
        }
    }
    QWidget::keyPressEvent(event);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────

void ExtractPage::loadArchiveFile(const QString& path) {
    // Setting text triggers onArchivePathChanged via textChanged signal
    m_archiveInput->setText(path);
}

void ExtractPage::onArchivePathChanged(const QString& path) {
    if (path.trimmed().isEmpty()) return;

    // Auto-fill destination only if the user hasn't typed one yet
    if (m_destInput->text().trimmed().isEmpty()) {
        QFileInfo fi(path.trimmed());
        if (fi.exists()) {
            const QString defaultDest =
                QString("%1/%2_extracted").arg(fi.absolutePath(), fi.completeBaseName());
            m_destInput->setText(defaultDest);
        }
    }

    // Inspect archive metadata (GCF native only)
    const QString lower = path.toLower().trimmed();
    if (lower.endsWith(".gcf") && QFile::exists(path.trimmed())) {
        inspectArchive(path.trimmed());
    } else {
        m_previewTable->setRowCount(0);
    }
}

void ExtractPage::onToggleDetails() {
    m_detailsVisible = !m_detailsVisible;
    m_detailsPanel->setVisible(m_detailsVisible);
    m_detailsToggleBtn->setText(m_detailsVisible ? "Hide Details" : "Show Details");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Slots - Browse
// ─────────────────────────────────────────────────────────────────────────────

void ExtractPage::onBrowseArchive() {
    const QString filter =
        "Compressed Archives (*.gcf *.zip *.rar *.7z *.tar.gz *.tar *.gz *.bz2 *.xz *.arc);;"
        "GCF Archive (*.gcf);;"
        "ZIP Archive (*.zip);;"
        "RAR Archive (*.rar);;"
        "7-Zip Archive (*.7z);;"
        "All Files (*.*)";

    const QString file =
        QFileDialog::getOpenFileName(this, "Select Archive File", QString(), filter);

    if (!file.isEmpty()) {
        m_archiveInput->setText(file);
        // Destination auto-fill only if still empty
        if (m_destInput->text().trimmed().isEmpty()) {
            QFileInfo fi(file);
            m_destInput->setText(
                QString("%1/%2_extracted").arg(fi.absolutePath(), fi.completeBaseName()));
        }
    }
}

void ExtractPage::onBrowseDestination() {
    const QString dir = QFileDialog::getExistingDirectory(this, "Select Output Folder");
    if (!dir.isEmpty()) {
        m_destInput->setText(dir);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Archive Inspection (GCF native format)
// ─────────────────────────────────────────────────────────────────────────────

void ExtractPage::inspectArchive(const QString& archivePath) {
    m_previewTable->setRowCount(0);
    try {
        gcore::Archive archive;
        const auto entries = archive.list_entries(archivePath.toStdString());

        m_logPanel->logInfo(
            QString("Inspecting archive: %1  (%2 entries)")
                .arg(archivePath)
                .arg(entries.size()));

        for (const auto& entry : entries) {
            const int row = m_previewTable->rowCount();
            m_previewTable->insertRow(row);

            auto* pathItem = new QTableWidgetItem(
                QString::fromStdString(entry.relative_path));
            auto* origSizeItem = new QTableWidgetItem(
                entry.is_directory ? "<DIR>" : QString("%1 B").arg(entry.original_size));
            auto* compSizeItem = new QTableWidgetItem(
                entry.is_directory ? "-" : QString("%1 B").arg(entry.compressed_size));
            auto* methodItem = new QTableWidgetItem(
                entry.is_directory ? "-"
                    : (entry.method == gcore::CompressionMethod::Deflate ? "DEFLATE" : "RAW"));
            auto* crcItem = new QTableWidgetItem(
                entry.is_directory ? "-"
                    : QString("0x%1").arg(entry.crc32, 8, 16, QChar('0')));

            origSizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            compSizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            methodItem->setTextAlignment(Qt::AlignCenter);
            crcItem->setTextAlignment(Qt::AlignCenter);

            m_previewTable->setItem(row, 0, pathItem);
            m_previewTable->setItem(row, 1, origSizeItem);
            m_previewTable->setItem(row, 2, compSizeItem);
            m_previewTable->setItem(row, 3, methodItem);
            m_previewTable->setItem(row, 4, crcItem);
        }

        m_statusLabel->setText(
            QString("Ready - %1 entries loaded. Click \"Show Details\" to inspect.")
                .arg(entries.size()));

    } catch (const std::exception& ex) {
        m_logPanel->logError(
            QString("Failed to read archive header: %1").arg(ex.what()));
        m_statusLabel->setText("Could not read archive metadata.");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Extraction
// ─────────────────────────────────────────────────────────────────────────────

void ExtractPage::onStartExtract() {
    const QString src = m_archiveInput->text().trimmed();
    QString dst = m_destInput->text().trimmed();

    if (src.isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
            "Please select a source archive first.");
        return;
    }

    // Auto-derive destination if still blank
    if (dst.isEmpty()) {
        QFileInfo fi(src);
        dst = QString("%1/%2_extracted").arg(fi.absolutePath(), fi.completeBaseName());
        m_destInput->setText(dst);
    }

    if (!QFile::exists(src)) {
        QMessageBox::critical(this, "Error", "The specified archive file does not exist.");
        return;
    }

    m_startBtn->setEnabled(false);
    m_browseArchiveBtn->setEnabled(false);
    m_browseDestBtn->setEnabled(false);
    m_progressBar->setValue(0);
    m_statusLabel->setText("Decompressing archive and verifying CRC-32 checksums...");
    m_logPanel->logInfo("Launching background extraction worker thread...");

    // Worker Thread
    m_workerThread = new QThread();
    m_worker = new gcontroller::CompressionWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, [this, src, dst]() {
        QMetaObject::invokeMethod(m_worker, "doExtract",
            Qt::QueuedConnection,
            Q_ARG(QString, src),
            Q_ARG(QString, dst));
    });

    connect(m_worker, &gcontroller::CompressionWorker::progress,
            this, &ExtractPage::onWorkerProgress);
    connect(m_worker, &gcontroller::CompressionWorker::logMessage,
            m_logPanel, &LogPanel::appendLog);
    connect(m_worker, &gcontroller::CompressionWorker::finished,
            this, &ExtractPage::onWorkerFinished);

    connect(m_worker, &gcontroller::CompressionWorker::finished,
            m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
    connect(m_workerThread, &QThread::destroyed, this, [this]() {
        m_workerThread = nullptr;
        m_worker = nullptr;
    });

    m_workerThread->start();
}

void ExtractPage::onWorkerProgress(const QString& currentFile,
                                   quint64 processed, quint64 total) {
    int percent = 0;
    if (total > 0) {
        percent = static_cast<int>((processed * 100) / total);
    }
    m_progressBar->setValue(percent);
    m_statusLabel->setText(
        QString("Unpacking: %1  (%2%)").arg(currentFile).arg(percent));
}

void ExtractPage::onWorkerFinished(bool success, const QString& message) {
    m_startBtn->setEnabled(true);
    m_browseArchiveBtn->setEnabled(true);
    m_browseDestBtn->setEnabled(true);

    if (success) {
        m_progressBar->setValue(100);
        m_statusLabel->setText("Extraction and verification successful!");
        QMessageBox::information(this, "Extraction Success", message);
    } else {
        m_statusLabel->setText("Extraction failed.");
        QMessageBox::critical(this, "Extraction Error", message);
    }
}

}  // namespace ggui

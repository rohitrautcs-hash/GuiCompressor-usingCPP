#include "compresspage.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QDir>
#include <QFileInfo>

namespace ggui {

CompressPage::CompressPage(QWidget* parent)
    : QWidget(parent) {
    setupUi();
}

CompressPage::~CompressPage() {
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
}

void CompressPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 24, 28, 24);
    mainLayout->setSpacing(18);

    // Title section
    auto* titleLabel = new QLabel("FOLDER COMPRESSION", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #00f0ff; letter-spacing: 1.5px;");
    
    auto* descLabel = new QLabel("Pack and compress local folders into GCF1 format with DEFLATE & CRC-32 integrity.", this);
    descLabel->setStyleSheet("color: #888899; font-size: 13px;");

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(descLabel);

    // Input Group Box / Card
    auto* cardWidget = new QWidget(this);
    cardWidget->setStyleSheet("background-color: #14141e; border: 1px solid #1a1a2e; border-radius: 10px;");
    auto* cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setContentsMargins(20, 20, 20, 20);
    cardLayout->setSpacing(14);

    // Source Folder Input Row
    auto* sourceLabel = new QLabel("Source Directory:", cardWidget);
    sourceLabel->setStyleSheet("color: #cccccc; font-weight: 600; font-size: 13px;");

    auto* sourceRow = new QHBoxLayout();
    m_sourceInput = new QLineEdit(cardWidget);
    m_sourceInput->setPlaceholderText("Select folder to compress...");
    m_sourceInput->setFixedHeight(38);
    m_sourceInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #1a1a24;
            border: 1px solid #2a2a3a;
            border-radius: 6px;
            color: #ffffff;
            padding: 0 12px;
        }
        QLineEdit:focus { border-color: #00f0ff; }
    )");

    m_browseSourceBtn = new QPushButton("Browse...", cardWidget);
    m_browseSourceBtn->setFixedSize(100, 38);
    m_browseSourceBtn->setCursor(Qt::PointingHandCursor);
    m_browseSourceBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #1f1f2e;
            color: #00f0ff;
            border: 1px solid #00f0ff;
            border-radius: 6px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #00f0ff;
            color: #000000;
        }
    )");
    connect(m_browseSourceBtn, &QPushButton::clicked, this, &CompressPage::onBrowseSource);

    sourceRow->addWidget(m_sourceInput);
    sourceRow->addWidget(m_browseSourceBtn);

    // Target Archive Input Row & Extension Selector
    auto* outputLabel = new QLabel("Output Archive & Preferred Extension:", cardWidget);
    outputLabel->setStyleSheet("color: #cccccc; font-weight: 600; font-size: 13px;");

    auto* outputRow = new QHBoxLayout();
    m_outputInput = new QLineEdit(cardWidget);
    m_outputInput->setPlaceholderText("Target output archive path...");
    m_outputInput->setFixedHeight(38);
    m_outputInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #1a1a24;
            border: 1px solid #2a2a3a;
            border-radius: 6px;
            color: #ffffff;
            padding: 0 12px;
        }
        QLineEdit:focus { border-color: #00f0ff; }
    )");

    m_extensionCombo = new QComboBox(cardWidget);
    m_extensionCombo->setFixedHeight(38);
    m_extensionCombo->setFixedWidth(110);
    m_extensionCombo->addItems({".gcf", ".rar", ".zip", ".7z", ".tar.gz", ".arc"});
    m_extensionCombo->setCursor(Qt::PointingHandCursor);
    m_extensionCombo->setStyleSheet(R"(
        QComboBox {
            background-color: #1f1f2e;
            color: #00f0ff;
            border: 1px solid #00f0ff;
            border-radius: 6px;
            padding: 0 8px;
            font-weight: bold;
            font-size: 13px;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox QAbstractItemView {
            background-color: #14141e;
            color: #ffffff;
            selection-background-color: #00f0ff;
            selection-color: #000000;
            border: 1px solid #00f0ff;
        }
    )");
    connect(m_extensionCombo, &QComboBox::currentTextChanged, this, &CompressPage::onExtensionChanged);

    m_browseOutputBtn = new QPushButton("Browse...", cardWidget);
    m_browseOutputBtn->setFixedSize(100, 38);
    m_browseOutputBtn->setCursor(Qt::PointingHandCursor);
    m_browseOutputBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #1f1f2e;
            color: #00f0ff;
            border: 1px solid #00f0ff;
            border-radius: 6px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #00f0ff;
            color: #000000;
        }
    )");
    connect(m_browseOutputBtn, &QPushButton::clicked, this, &CompressPage::onBrowseOutput);

    outputRow->addWidget(m_outputInput, 1);
    outputRow->addWidget(m_extensionCombo);
    outputRow->addWidget(m_browseOutputBtn);

    cardLayout->addWidget(sourceLabel);
    cardLayout->addLayout(sourceRow);
    cardLayout->addWidget(outputLabel);
    cardLayout->addLayout(outputRow);

    mainLayout->addWidget(cardWidget);

    // Progress Section
    m_statusLabel = new QLabel("Ready to compress.", this);
    m_statusLabel->setStyleSheet("color: #aaaaaa; font-size: 12px; font-weight: 500;");

    m_progressBar = new QProgressBar(this);
    m_progressBar->setFixedHeight(14);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(R"(
        QProgressBar {
            background-color: #1a1a24;
            border: 1px solid #2a2a3a;
            border-radius: 7px;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f0ff, stop:1 #39ff14);
            border-radius: 6px;
        }
    )");

    // Action Button
    m_startBtn = new QPushButton("⚡ START COMPRESSION", this);
    m_startBtn->setFixedHeight(44);
    m_startBtn->setCursor(Qt::PointingHandCursor);
    m_startBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f0ff, stop:1 #00a8ff);
            color: #000000;
            font-size: 14px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
            letter-spacing: 1px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #39ff14, stop:1 #00f0ff);
        }
        QPushButton:disabled {
            background-color: #222230;
            color: #555566;
        }
    )");

    connect(m_startBtn, &QPushButton::clicked, this, &CompressPage::onStartCompress);

    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addWidget(m_startBtn);

    // Log Console Panel
    m_logPanel = new LogPanel(this);
    mainLayout->addWidget(m_logPanel, 1);
}

void CompressPage::onBrowseSource() {
    const QString dir = QFileDialog::getExistingDirectory(this, "Select Source Directory");
    if (!dir.isEmpty()) {
        m_sourceInput->setText(dir);
        if (m_outputInput->text().isEmpty()) {
            QDir d(dir);
            const QString selectedExt = m_extensionCombo ? m_extensionCombo->currentText() : ".gcf";
            const QString defaultArc = QString("%1/%2%3").arg(d.absolutePath(), d.dirName(), selectedExt);
            m_outputInput->setText(defaultArc);
        }
    }
}

void CompressPage::onExtensionChanged(const QString& ext) {
    QString currentPath = m_outputInput->text().trimmed();
    if (currentPath.isEmpty()) return;

    // Strip existing extension if it ends with one of our known extensions
    const QStringList knownExts = {".gcf", ".rar", ".zip", ".7z", ".tar.gz", ".arc"};
    for (const QString& kExt : knownExts) {
        if (currentPath.endsWith(kExt, Qt::CaseInsensitive)) {
            currentPath.chop(kExt.length());
            break;
        }
    }
    m_outputInput->setText(currentPath + ext);
}

void CompressPage::onBrowseOutput() {
    const QString filter = "GCF Archive (*.gcf);;RAR Archive (*.rar);;ZIP Archive (*.zip);;7-Zip Archive (*.7z);;Tarball (*.tar.gz);;All Files (*.*)";
    const QString file = QFileDialog::getSaveFileName(this, "Save Archive As", "", filter);
    if (!file.isEmpty()) {
        m_outputInput->setText(file);
        // Automatically sync combo box extension with chosen filename
        const QStringList knownExts = {".gcf", ".rar", ".zip", ".7z", ".tar.gz", ".arc"};
        for (const QString& kExt : knownExts) {
            if (file.endsWith(kExt, Qt::CaseInsensitive)) {
                int idx = m_extensionCombo->findText(kExt);
                if (idx != -1) {
                    m_extensionCombo->setCurrentIndex(idx);
                }
                break;
            }
        }
    }
}

void CompressPage::onStartCompress() {
    const QString src = m_sourceInput->text().trimmed();
    const QString dst = m_outputInput->text().trimmed();

    if (src.isEmpty() || dst.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please select both source folder and target output path.");
        return;
    }

    if (!QDir(src).exists()) {
        QMessageBox::critical(this, "Error", "The specified source directory does not exist.");
        return;
    }

    m_startBtn->setEnabled(false);
    m_browseSourceBtn->setEnabled(false);
    m_browseOutputBtn->setEnabled(false);
    m_progressBar->setValue(0);
    m_statusLabel->setText("Compressing folder into .gcf archive...");
    m_logPanel->logInfo("Starting compression task thread...");

    // Worker Thread Setup
    m_workerThread = new QThread();
    m_worker = new gcontroller::CompressionWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, [this, src, dst]() {
        QMetaObject::invokeMethod(m_worker, "doCompress", Qt::QueuedConnection, Q_ARG(QString, src), Q_ARG(QString, dst));
    });

    connect(m_worker, &gcontroller::CompressionWorker::progress, this, &CompressPage::onWorkerProgress);
    connect(m_worker, &gcontroller::CompressionWorker::logMessage, m_logPanel, &LogPanel::appendLog);
    connect(m_worker, &gcontroller::CompressionWorker::finished, this, &CompressPage::onWorkerFinished);

    connect(m_worker, &gcontroller::CompressionWorker::finished, m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);
    connect(m_workerThread, &QThread::destroyed, this, [this]() {
        m_workerThread = nullptr;
        m_worker = nullptr;
    });

    m_workerThread->start();
}

void CompressPage::onWorkerProgress(const QString& currentFile, quint64 processed, quint64 total) {
    int percent = 0;
    if (total > 0) {
        percent = static_cast<int>((processed * 100) / total);
    }
    m_progressBar->setValue(percent);
    m_statusLabel->setText(QString("Processing: %1 (%2%)").arg(currentFile).arg(percent));
}

void CompressPage::onWorkerFinished(bool success, const QString& message) {
    m_startBtn->setEnabled(true);
    m_browseSourceBtn->setEnabled(true);
    m_browseOutputBtn->setEnabled(true);

    if (success) {
        m_progressBar->setValue(100);
        m_statusLabel->setText("Compression complete!");
        QMessageBox::information(this, "Compression Success", message);
    } else {
        m_statusLabel->setText("Compression failed.");
        QMessageBox::critical(this, "Compression Error", message);
    }
}

}  // namespace ggui

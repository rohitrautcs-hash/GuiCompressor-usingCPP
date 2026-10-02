#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QThread>

#include "logpanel.hpp"
#include "../controller/worker.hpp"

#include <QComboBox>

namespace ggui {

/**
 * @brief Compress Page for folder-to-archive generation.
 * 
 * Manages source folder picking, destination selection, extension selection (.gcf, .rar, .zip, etc.),
 * progress bar updates, background thread delegation, and console log output.
 */
class CompressPage : public QWidget {
    Q_OBJECT

public:
    explicit CompressPage(QWidget* parent = nullptr);
    ~CompressPage() override;

private slots:
    void onBrowseSource();
    void onBrowseOutput();
    void onExtensionChanged(const QString& ext);
    void onStartCompress();
    void onWorkerProgress(const QString& currentFile, quint64 processed, quint64 total);
    void onWorkerFinished(bool success, const QString& message);

private:
    void setupUi();

    QLineEdit* m_sourceInput{nullptr};
    QPushButton* m_browseSourceBtn{nullptr};

    QLineEdit* m_outputInput{nullptr};
    QComboBox* m_extensionCombo{nullptr};
    QPushButton* m_browseOutputBtn{nullptr};

    QPushButton* m_startBtn{nullptr};
    QProgressBar* m_progressBar{nullptr};
    QLabel* m_statusLabel{nullptr};

    LogPanel* m_logPanel{nullptr};

    QThread* m_workerThread{nullptr};
    gcontroller::CompressionWorker* m_worker{nullptr};
};

}  // namespace ggui

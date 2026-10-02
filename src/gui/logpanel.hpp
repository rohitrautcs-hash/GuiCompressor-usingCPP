#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>

namespace ggui {

/**
 * @brief Terminal-style Log Console Panel shared across Compress and Extract views.
 * 
 * Renders color-coded timestamped messages with auto-scrolling, clipboard copy,
 * log clearing, and line buffer cap (max 500 lines).
 */
class LogPanel : public QWidget {
    Q_OBJECT

public:
    explicit LogPanel(QWidget* parent = nullptr);
    ~LogPanel() override = default;

public slots:
    void logInfo(const QString& message);
    void logSuccess(const QString& message);
    void logWarning(const QString& message);
    void logError(const QString& message);
    void appendLog(const QString& level, const QString& message);
    void clearLogs();
    void copyToClipboard();

private:
    void setupUi();
    void trimLogsIfNeeded();

    QTextEdit* m_consoleDisplay{nullptr};
    QPushButton* m_copyButton{nullptr};
    QPushButton* m_clearButton{nullptr};
    int m_lineCount{0};
};

}  // namespace ggui

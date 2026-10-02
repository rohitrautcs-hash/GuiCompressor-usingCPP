#include "logpanel.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTime>
#include <QGuiApplication>
#include <QClipboard>
#include <QTextCursor>
#include <QScrollBar>

namespace ggui {

LogPanel::LogPanel(QWidget* parent)
    : QWidget(parent) {
    setupUi();
}

void LogPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Panel Header Bar
    auto* headerWidget = new QWidget(this);
    headerWidget->setStyleSheet("background-color: #0c0c14; border-top: 1px solid #1a1a2e; border-bottom: 1px solid #1a1a2e;");
    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(12, 6, 12, 6);

    auto* titleLabel = new QLabel("CONSOLE LOG", headerWidget);
    titleLabel->setStyleSheet("font-family: 'JetBrains Mono', Consolas, monospace; font-size: 11px; font-weight: bold; color: #00f0ff; letter-spacing: 1px;");

    m_copyButton = new QPushButton("COPY", headerWidget);
    m_copyButton->setCursor(Qt::PointingHandCursor);
    m_copyButton->setStyleSheet(R"(
        QPushButton {
            background-color: #1a1a24;
            color: #cccccc;
            border: 1px solid #2a2a3a;
            border-radius: 4px;
            font-size: 10px;
            font-weight: bold;
            padding: 3px 8px;
        }
        QPushButton:hover {
            border-color: #00f0ff;
            color: #00f0ff;
        }
    )");

    m_clearButton = new QPushButton("CLEAR", headerWidget);
    m_clearButton->setCursor(Qt::PointingHandCursor);
    m_clearButton->setStyleSheet(R"(
        QPushButton {
            background-color: #1a1a24;
            color: #cccccc;
            border: 1px solid #2a2a3a;
            border-radius: 4px;
            font-size: 10px;
            font-weight: bold;
            padding: 3px 8px;
        }
        QPushButton:hover {
            border-color: #ff4444;
            color: #ff4444;
        }
    )");

    connect(m_copyButton, &QPushButton::clicked, this, &LogPanel::copyToClipboard);
    connect(m_clearButton, &QPushButton::clicked, this, &LogPanel::clearLogs);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_copyButton);
    headerLayout->addWidget(m_clearButton);

    // Terminal Display
    m_consoleDisplay = new QTextEdit(this);
    m_consoleDisplay->setReadOnly(true);
    m_consoleDisplay->setUndoRedoEnabled(false);
    m_consoleDisplay->setMinimumHeight(120);
    m_consoleDisplay->setStyleSheet(R"(
        QTextEdit {
            background-color: #08080c;
            color: #cccccc;
            border: none;
            font-family: 'JetBrains Mono', Consolas, 'Courier New', monospace;
            font-size: 12px;
            line-height: 1.4;
            padding: 8px;
        }
    )");

    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(m_consoleDisplay);
}

void LogPanel::logInfo(const QString& message) {
    appendLog("INFO", message);
}

void LogPanel::logSuccess(const QString& message) {
    appendLog("SUCCESS", message);
}

void LogPanel::logWarning(const QString& message) {
    appendLog("WARNING", message);
}

void LogPanel::logError(const QString& message) {
    appendLog("ERROR", message);
}

void LogPanel::appendLog(const QString& level, const QString& message) {
    const QString timestamp = QTime::currentTime().toString("[hh:mm:ss]");
    QString color = "#cccccc";

    if (level == "INFO") {
        color = "#00f0ff";
    } else if (level == "SUCCESS") {
        color = "#39ff14";
    } else if (level == "WARNING") {
        color = "#ffb800";
    } else if (level == "ERROR") {
        color = "#ff4444";
    }

    const QString htmlLine = QString("<span style='color:#666666;'>%1</span> "
                                    "<span style='color:%2; font-weight:bold;'>[%3]</span> "
                                    "<span style='color:#ffffff;'>%4</span>")
                                 .arg(timestamp, color, level, message.toHtmlEscaped());

    m_consoleDisplay->append(htmlLine);
    m_lineCount++;

    trimLogsIfNeeded();

    // Auto scroll to bottom
    QScrollBar* sb = m_consoleDisplay->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void LogPanel::clearLogs() {
    m_consoleDisplay->clear();
    m_lineCount = 0;
}

void LogPanel::copyToClipboard() {
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(m_consoleDisplay->toPlainText());
}

void LogPanel::trimLogsIfNeeded() {
    if (m_lineCount > 500) {
        QTextCursor cursor(m_consoleDisplay->document());
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, 50);
        cursor.removeSelectedText();
        m_lineCount -= 50;
    }
}

}  // namespace ggui

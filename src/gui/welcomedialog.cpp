#include "welcomedialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QSettings>
#include <QPropertyAnimation>
#include <QStyleOption>
#include <QPainter>

namespace ggui {

WelcomeDialog::WelcomeDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Welcome to GUI Compressor v1.0");
    setFixedSize(540, 420);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setupUi();

    m_fullHeadline = "Experience Next-Gen Lossless Folder Archiving";
    m_typewriterIndex = 0;
    m_headlineLabel->setText("");

    m_typewriterTimer = new QTimer(this);
    connect(m_typewriterTimer, &QTimer::timeout, this, &WelcomeDialog::onTypewriterTick);
    m_typewriterTimer->start(35);
}

bool WelcomeDialog::shouldShowWelcome() {
    QSettings settings("KathmanduDonBosco", "GUICompressor");
    return !settings.value("has_seen_welcome", false).toBool();
}

QString WelcomeDialog::getSavedUserName() {
    QSettings settings("KathmanduDonBosco", "GUICompressor");
    return settings.value("user_name", "User").toString();
}

void WelcomeDialog::resetWelcomeSetting() {
    QSettings settings("KathmanduDonBosco", "GUICompressor");
    settings.setValue("has_seen_welcome", false);
}

void WelcomeDialog::setupUi() {
    QWidget* container = new QWidget(this);
    container->setObjectName("dialogContainer");
    container->setStyleSheet(R"(
        QWidget#dialogContainer {
            background-color: #0a0a12;
            border: 2px solid #00f0ff;
            border-radius: 16px;
        }
    )");

    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(10, 10, 10, 10);
    outerLayout->addWidget(container);

    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(36, 36, 36, 36);
    layout->setSpacing(16);

    // Header Logo Icon
    m_logoLabel = new QLabel("⚡ GUI COMPRESSOR v1.0", container);
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setStyleSheet(R"(
        font-family: 'Segoe UI', sans-serif;
        font-size: 22px;
        font-weight: bold;
        color: #00f0ff;
        letter-spacing: 2px;
    )");

    // Drop shadow glow effect for logo
    auto* logoGlow = new QGraphicsDropShadowEffect(this);
    logoGlow->setBlurRadius(20);
    logoGlow->setColor(QColor(0, 240, 255, 180));
    logoGlow->setOffset(0, 0);
    m_logoLabel->setGraphicsEffect(logoGlow);

    // Headline with Typewriter Effect
    m_headlineLabel = new QLabel(container);
    m_headlineLabel->setAlignment(Qt::AlignCenter);
    m_headlineLabel->setWordWrap(true);
    m_headlineLabel->setStyleSheet(R"(
        font-family: 'Segoe UI', sans-serif;
        font-size: 15px;
        color: #cccccc;
        font-weight: 500;
    )");

    m_subtextLabel = new QLabel(" Kathmandu Don Bosco College • BIT 2nd Semester Project", container);
    m_subtextLabel->setAlignment(Qt::AlignCenter);
    m_subtextLabel->setStyleSheet("color: #666666; font-size: 11px; font-style: italic;");

    // Input section
    QLabel* namePrompt = new QLabel("What should we call you?", container);
    namePrompt->setStyleSheet("color: #00f0ff; font-size: 13px; font-weight: bold; margin-top: 10px;");

    m_nameInput = new QLineEdit(container);
    m_nameInput->setPlaceholderText("Enter your name (e.g. Rohit)");
    m_nameInput->setText(getSavedUserName());
    m_nameInput->setFixedHeight(42);
    m_nameInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #1a1a24;
            border: 1.5px solid #2a2a3a;
            border-radius: 8px;
            color: #ffffff;
            padding: 0 12px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border: 1.5px solid #00f0ff;
            background-color: #222232;
        }
    )");

    // Start Button
    m_startButton = new QPushButton("GET STARTED  →", container);
    m_startButton->setFixedHeight(46);
    m_startButton->setCursor(Qt::PointingHandCursor);
    m_startButton->setStyleSheet(R"(
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
        QPushButton:pressed {
            background-color: #00c0cc;
        }
    )");

    auto* btnGlow = new QGraphicsDropShadowEffect(this);
    btnGlow->setBlurRadius(15);
    btnGlow->setColor(QColor(0, 240, 255, 120));
    btnGlow->setOffset(0, 2);
    m_startButton->setGraphicsEffect(btnGlow);

    connect(m_startButton, &QPushButton::clicked, this, &WelcomeDialog::onStartClicked);

    layout->addWidget(m_logoLabel);
    layout->addWidget(m_headlineLabel);
    layout->addWidget(m_subtextLabel);
    layout->addSpacing(8);
    layout->addWidget(namePrompt);
    layout->addWidget(m_nameInput);
    layout->addSpacing(10);
    layout->addWidget(m_startButton);
}

void WelcomeDialog::onTypewriterTick() {
    if (m_typewriterIndex < m_fullHeadline.length()) {
        m_headlineLabel->setText(m_fullHeadline.left(++m_typewriterIndex));
    } else {
        m_typewriterTimer->stop();
    }
}

void WelcomeDialog::onStartClicked() {
    QString name = m_nameInput->text().trimmed();
    if (name.isEmpty()) {
        name = "User";
    }

    QSettings settings("KathmanduDonBosco", "GUICompressor");
    settings.setValue("has_seen_welcome", true);
    settings.setValue("user_name", name);

    emit userNameEntered(name);
    accept();
}

}  // namespace ggui

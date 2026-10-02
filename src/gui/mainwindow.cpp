#include "mainwindow.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

#include "welcomedialog.hpp"

namespace ggui {

MainWindow::MainWindow(const QString& userName, QWidget* parent)
    : QMainWindow(parent), m_userName(userName) {
    setWindowTitle("GUI Compressor v1.0 — Lossless Folder Archiver");
    resize(1000, 680);
    setMinimumSize(850, 550);

    setupUi();
    applyGlobalStyle();

    setGreetingName(userName);
    switchToPage(0);
}

void MainWindow::setGreetingName(const QString& name) {
    m_userName = name;
    if (m_greetingLabel) {
        m_greetingLabel->setText(QString("Hello, %1 👋").arg(name));
    }
}

void MainWindow::setupUi() {
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Left Navigation Sidebar (180px)
    auto* sidebarWidget = new QWidget(centralWidget);
    sidebarWidget->setFixedWidth(200);
    sidebarWidget->setObjectName("sidebarWidget");

    auto* sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(16, 24, 16, 24);
    sidebarLayout->setSpacing(10);

    // Brand Label
    auto* brandLabel = new QLabel("GUI COMPRESSOR", sidebarWidget);
    brandLabel->setStyleSheet("font-size: 14px; font-weight: 900; color: #00f0ff; letter-spacing: 2px;");

    auto* versionLabel = new QLabel("v1.0 • Multi-Format", sidebarWidget);
    versionLabel->setStyleSheet("font-size: 10px; color: #666666; font-weight: bold; margin-bottom: 6px;");

    // Personalized User Greeting Label
    m_greetingLabel = new QLabel(sidebarWidget);
    m_greetingLabel->setWordWrap(true);
    m_greetingLabel->setStyleSheet("font-size: 12px; font-weight: 600; color: #39ff14; background: #141420; padding: 8px 10px; border-radius: 6px; border: 1px solid #1a1a2e;");

    // Edit Name / Change User Button
    auto* changeUserBtn = new QPushButton("✏️ Change User Name", sidebarWidget);
    changeUserBtn->setCursor(Qt::PointingHandCursor);
    changeUserBtn->setStyleSheet(R"(
        QPushButton {
            color: #8888aa;
            font-size: 10px;
            font-weight: 600;
            background-color: transparent;
            border: 1px solid #222233;
            border-radius: 4px;
            padding: 4px 6px;
            margin-bottom: 12px;
        }
        QPushButton:hover {
            color: #00f0ff;
            border-color: #00f0ff;
            background-color: #141424;
        }
    )");

    connect(changeUserBtn, &QPushButton::clicked, this, [this]() {
        WelcomeDialog welcome(this);
        connect(&welcome, &WelcomeDialog::userNameEntered, this, &MainWindow::setGreetingName);
        welcome.exec();
    });

    // Navigation Buttons
    m_compressNavBtn = new QPushButton("📦 COMPRESS", sidebarWidget);
    m_extractNavBtn  = new QPushButton("📂 EXTRACT", sidebarWidget);
    m_aboutNavBtn    = new QPushButton("ℹ️ ABOUT", sidebarWidget);

    const QString btnStyle = R"(
        QPushButton {
            text-align: left;
            padding: 12px 14px;
            font-size: 12px;
            font-weight: bold;
            color: #888899;
            background-color: transparent;
            border: 1px solid transparent;
            border-radius: 8px;
            letter-spacing: 1px;
        }
        QPushButton:hover {
            color: #00f0ff;
            background-color: #141422;
            border: 1px solid #1a1a30;
        }
        QPushButton:checked {
            color: #00f0ff;
            background-color: #1a1a2e;
            border: 1px solid #00f0ff;
        }
    )";

    m_compressNavBtn->setCheckable(true);
    m_extractNavBtn->setCheckable(true);
    m_aboutNavBtn->setCheckable(true);

    m_compressNavBtn->setStyleSheet(btnStyle);
    m_extractNavBtn->setStyleSheet(btnStyle);
    m_aboutNavBtn->setStyleSheet(btnStyle);

    m_compressNavBtn->setCursor(Qt::PointingHandCursor);
    m_extractNavBtn->setCursor(Qt::PointingHandCursor);
    m_aboutNavBtn->setCursor(Qt::PointingHandCursor);

    connect(m_compressNavBtn, &QPushButton::clicked, [this]() { switchToPage(0); });
    connect(m_extractNavBtn,  &QPushButton::clicked, [this]() { switchToPage(1); });
    connect(m_aboutNavBtn,    &QPushButton::clicked, [this]() { switchToPage(2); });

    sidebarLayout->addWidget(brandLabel);
    sidebarLayout->addWidget(versionLabel);
    sidebarLayout->addWidget(m_greetingLabel);
    sidebarLayout->addWidget(changeUserBtn);
    sidebarLayout->addSpacing(8);
    sidebarLayout->addWidget(m_compressNavBtn);
    sidebarLayout->addWidget(m_extractNavBtn);
    sidebarLayout->addWidget(m_aboutNavBtn);
    sidebarLayout->addStretch();

    // Footer copyright
    auto* footerLabel = new QLabel("Kathmandu Don Bosco\nBIT 2nd Semester", sidebarWidget);
    footerLabel->setStyleSheet("color: #444455; font-size: 10px; text-align: center;");
    sidebarLayout->addWidget(footerLabel);

    // Main Content Area Stack
    m_pageStack = new QStackedWidget(centralWidget);
    m_compressPage = new CompressPage(m_pageStack);
    m_extractPage  = new ExtractPage(m_pageStack);
    m_aboutPage    = new AboutPage(m_pageStack);

    m_pageStack->addWidget(m_compressPage);
    m_pageStack->addWidget(m_extractPage);
    m_pageStack->addWidget(m_aboutPage);

    mainLayout->addWidget(sidebarWidget);
    mainLayout->addWidget(m_pageStack, 1);
}

void MainWindow::switchToPage(int index) {
    m_compressNavBtn->setChecked(index == 0);
    m_extractNavBtn->setChecked(index == 1);
    m_aboutNavBtn->setChecked(index == 2);

    if (m_pageStack->currentIndex() != index) {
        QWidget* targetPage = m_pageStack->widget(index);
        m_pageStack->setCurrentIndex(index);

        // Smooth Opacity Fade-In Animation
        auto* effect = new QGraphicsOpacityEffect(targetPage);
        targetPage->setGraphicsEffect(effect);

        auto* anim = new QPropertyAnimation(effect, "opacity", targetPage);
        anim->setDuration(220);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::InOutQuad);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::applyGlobalStyle() {
    setStyleSheet(R"(
        QMainWindow {
            background-color: #0f0f16;
        }
        QWidget#sidebarWidget {
            background-color: #0c0c14;
            border-right: 1px solid #1a1a2e;
        }
        QScrollBar:vertical {
            background-color: #08080c;
            width: 8px;
            border: none;
        }
        QScrollBar::handle:vertical {
            background-color: #1f1f2e;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #00f0ff;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");
}

}  // namespace ggui

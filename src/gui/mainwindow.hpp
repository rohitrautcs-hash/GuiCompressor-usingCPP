#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QString>

#include "compresspage.hpp"
#include "extractpage.hpp"
#include "aboutpage.hpp"

namespace ggui {

/**
 * @brief Main Application Window for GUI Compressor v1.0.
 * 
 * Houses the left navigation sidebar with user greeting, QStackedWidget container,
 * page transition animations, and dark cyberpunk global styling.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const QString& userName = "User", QWidget* parent = nullptr);
    ~MainWindow() override = default;

public slots:
    void setGreetingName(const QString& name);
    void switchToPage(int index);

private:
    void setupUi();
    void applyGlobalStyle();

    QString m_userName;
    QLabel* m_greetingLabel{nullptr};

    QPushButton* m_compressNavBtn{nullptr};
    QPushButton* m_extractNavBtn{nullptr};
    QPushButton* m_aboutNavBtn{nullptr};

    QStackedWidget* m_pageStack{nullptr};
    CompressPage* m_compressPage{nullptr};
    ExtractPage* m_extractPage{nullptr};
    AboutPage* m_aboutPage{nullptr};
};

}  // namespace ggui

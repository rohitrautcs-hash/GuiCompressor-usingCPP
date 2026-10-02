#include <QApplication>
#include <QPalette>
#include <QColor>

#include "gui/welcomedialog.hpp"
#include "gui/mainwindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("GUI Compressor");
    app.setOrganizationName("Kathmandu Don Bosco College");
    app.setApplicationVersion("1.0");

    // Set Dark Qt Palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(15, 15, 22));
    darkPalette.setColor(QPalette::WindowText, QColor(204, 204, 204));
    darkPalette.setColor(QPalette::Base, QColor(26, 26, 36));
    darkPalette.setColor(QPalette::AlternateBase, QColor(20, 20, 30));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(0, 240, 255));
    darkPalette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
    darkPalette.setColor(QPalette::Text, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Button, QColor(26, 26, 36));
    darkPalette.setColor(QPalette::ButtonText, QColor(204, 204, 204));
    darkPalette.setColor(QPalette::BrightText, QColor(57, 255, 20));
    darkPalette.setColor(QPalette::Highlight, QColor(0, 240, 255));
    darkPalette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
    app.setPalette(darkPalette);

    QString userName = ggui::WelcomeDialog::getSavedUserName();

    if (ggui::WelcomeDialog::shouldShowWelcome()) {
        ggui::WelcomeDialog welcome;
        QObject::connect(&welcome, &ggui::WelcomeDialog::userNameEntered, [&userName](const QString& name) {
            userName = name;
        });
        if (welcome.exec() != QDialog::Accepted) {
            return 0;
        }
    }

    ggui::MainWindow mainWindow(userName);
    mainWindow.show();

    return app.exec();
}

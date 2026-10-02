#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QString>

namespace ggui {

/**
 * @brief Animated Welcome Dialog displayed on first launch.
 * 
 * Prompts user for their name, stores it in QSettings, and presents
 * a personalized cyberpunk-themed greeting screen with typewriter animation.
 */
class WelcomeDialog : public QDialog {
    Q_OBJECT

public:
    explicit WelcomeDialog(QWidget* parent = nullptr);
    ~WelcomeDialog() override = default;

    /**
     * @brief Checks QSettings to see if user has already completed welcome onboarding.
     */
    static bool shouldShowWelcome();

    /**
     * @brief Retrieves stored username from QSettings (defaults to "User").
     */
    static QString getSavedUserName();

    /**
     * @brief Resets the has_seen_welcome flag in QSettings so the prompt reappears.
     */
    static void resetWelcomeSetting();

signals:
    void userNameEntered(const QString& name);

private slots:
    void onTypewriterTick();
    void onStartClicked();

private:
    void setupUi();

    QLabel* m_logoLabel{nullptr};
    QLabel* m_headlineLabel{nullptr};
    QLabel* m_subtextLabel{nullptr};
    QLineEdit* m_nameInput{nullptr};
    QPushButton* m_startButton{nullptr};

    QTimer* m_typewriterTimer{nullptr};
    QString m_fullHeadline;
    int m_typewriterIndex{0};
};

}  // namespace ggui

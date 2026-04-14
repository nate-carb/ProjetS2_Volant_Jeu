#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMovie>
#include "OptionsDialog.h"
#include "ControlsDialog.h"
#include "SoundManager.h"

class MenuWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MenuWindow(SoundManager* soundManager, QWidget* parent = nullptr);
    ~MenuWindow();
    void goToMainMenu() { goToPage(PAGE_MAIN); }

signals:
    void playRequested(int trackIndex);

private slots:
    void onPlay();
    void onLeaderboards();
    void onOptions();
    void onControls();
    void onTrackSelected(int trackIndex);
    void onLeaderboardTrackSelected(int trackIndex);
    void goToPage(int index);

private:
    QStackedWidget* m_stack;
    SoundManager* m_soundManager = nullptr;

    // Pages
    QWidget* createMainMenuPage();
    QWidget* createTrackSelectionPage();
    QWidget* createLeaderboardSelectionPage();
    QWidget* createLeaderboardPage(int trackIndex);

    // Helpers
    QPushButton* createStyledButton(const QString& text);
    QLabel* createTitle(const QString& text);
    void         applyBackground(QWidget* page);

    // GIF background
    QLabel* m_bgLabel = nullptr;
    QMovie* m_bgMovie = nullptr;
    void     setupBackground(QWidget* page);

    enum Pages {
        PAGE_MAIN = 0,
        PAGE_TRACK_SEL = 1,
        PAGE_LB_SEL = 2,
        PAGE_LB_T1 = 3,
        PAGE_LB_T2 = 4,
        PAGE_LB_T3 = 5,
    };
};
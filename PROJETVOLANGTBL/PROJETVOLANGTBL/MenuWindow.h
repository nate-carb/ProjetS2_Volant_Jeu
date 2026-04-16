#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMovie>
#include <QScrollBar>
#include <QShowEvent>
#include <QHideEvent>
#include "OptionsDialog.h"
#include "ControlsDialog.h"
#include "SoundManager.h"
#include "ArduinoManager.h"

// --- forward declarations ---
class ArduinoManager;
class QTimer;
class QScrollArea;
class QShowEvent;
class QHideEvent;

class MenuWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MenuWindow(SoundManager* soundManager, ArduinoManager* arduino, QWidget* parent = nullptr);
    ~MenuWindow();
    void goToMainMenu() { goToPage(PAGE_MAIN); }
    QWidget* getCurrentPage() const { return m_stack->currentWidget(); }
    QWidget* getPage(int index) const { return m_stack->widget(index); }

signals:
    void playRequested(int trackIndex);

private slots:
    void pollEncoder();
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

	// For communicating with the Arduino (encoder inputs)
    ArduinoManager* m_arduino = nullptr;
    QTimer* m_encoderTimer = nullptr;
    QScrollArea* m_lbScrolls[3] = { nullptr, nullptr, nullptr };
    int m_prevEnc2 = 0;
    bool m_prevEnc2Init = false;

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
protected:
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;
};
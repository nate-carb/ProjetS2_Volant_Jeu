#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "OptionsDialog.h"
#include "ControlsDialog.h"
#include "SoundManager.h"

class PauseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PauseDialog(SoundManager* soundManager = nullptr, QWidget* parent = nullptr);

    enum Result {
        RESUME,
        RESTART,
        MAIN_MENU
    };

    Result getResult() const { return m_result; }

private:
    Result m_result = RESUME;
    SoundManager* m_soundManager = nullptr;
    QPushButton* createStyledButton(const QString& text);
};
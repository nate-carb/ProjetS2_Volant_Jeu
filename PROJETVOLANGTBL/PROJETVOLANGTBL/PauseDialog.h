#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "OptionsDialog.h"
#include "ControlsDialog.h"

class PauseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PauseDialog(QWidget* parent = nullptr);

    enum Result {
        RESUME,
        RESTART,
        MAIN_MENU
    };

    Result getResult() const { return m_result; }

private:
    Result m_result = RESUME;
    QPushButton* createStyledButton(const QString& text);
};

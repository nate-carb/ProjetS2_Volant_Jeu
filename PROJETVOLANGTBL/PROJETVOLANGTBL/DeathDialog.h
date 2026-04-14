#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "AnimatedButton.h"

class DeathDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeathDialog(const QString& reason, QWidget* parent = nullptr);

    enum Result { RESTART, MAIN_MENU };
    Result getResult() const { return m_result; }

private:
    Result m_result = RESTART;
    QPushButton* createStyledButton(const QString& text);
};

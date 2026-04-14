#include "DeathDialog.h"

DeathDialog::DeathDialog(const QString& reason, QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Game Over");
    setFixedSize(400, 300);
    setStyleSheet("background: transparent;");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // Widget central avec coins ronds
    QWidget* centerWidget = new QWidget(this);
    centerWidget->setStyleSheet(
        "background-color: #1a0000;"
        "border-radius: 20px;");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(centerWidget);

    QVBoxLayout* layout = new QVBoxLayout(centerWidget);
    layout->setSpacing(15);
    layout->setContentsMargins(40, 30, 40, 30);

    // Titre GAME OVER
    QLabel* title = new QLabel("GAME OVER");
    title->setStyleSheet(
        "color: #FF0000; font-size: 36px; font-weight: bold;"
        "background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // Raison
    QLabel* reasonLabel = new QLabel(reason);
    reasonLabel->setStyleSheet(
        "color: white; font-size: 14px;"
        "background: transparent;");
    reasonLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(reasonLabel);

    layout->addSpacing(10);

    // Boutons
    QPushButton* btnRestart = createStyledButton("Recommencer");
    QPushButton* btnMainMenu = createStyledButton("Menu Principal");

    connect(btnRestart, &QPushButton::clicked, [this]() {
        m_result = RESTART;
        accept();
        });

    connect(btnMainMenu, &QPushButton::clicked, [this]() {
        m_result = MAIN_MENU;
        accept();
        });

    layout->addWidget(btnRestart);
    layout->addWidget(btnMainMenu);
}

QPushButton* DeathDialog::createStyledButton(const QString& text)
{
    AnimatedButton* btn = new AnimatedButton(text);
    btn->setFixedHeight(55);
    return btn;
}
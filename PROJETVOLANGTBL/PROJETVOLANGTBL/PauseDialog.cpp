#include "PauseDialog.h"

static const QString BTN_STYLE = R"(
    QPushButton {
        background-color: #111111;
        color: white;
        font-size: 22px;
        font-weight: bold;
        border-radius: 10px;
        padding: 14px;
        border: 2px solid #333;
    }
    QPushButton:hover {
        background-color: #CC0000;
        border: 2px solid #FF3333;
    }
    QPushButton:pressed {
        background-color: #880000;
    }
)";

PauseDialog::PauseDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Paused");
    setFixedSize(400, 420);
    setStyleSheet("background-color: #CC2200;");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(40, 30, 40, 30);

    // Titre PAUSED
    QLabel* title = new QLabel("PAUSED");
    title->setStyleSheet(
        "color: white; font-size: 36px; font-weight: bold;"
        "background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);
    layout->addSpacing(10);

    // Boutons
    QPushButton* btnResume = createStyledButton("Resume");
    QPushButton* btnRestart = createStyledButton("Restart");
    QPushButton* btnOptions = createStyledButton("Options");
    QPushButton* btnControls = createStyledButton("Controls");
    QPushButton* btnMainMenu = createStyledButton("Main Menu");

    // Resume
    connect(btnResume, &QPushButton::clicked, [this]() {
        m_result = RESUME;
        accept();
        });

    // Restart
    connect(btnRestart, &QPushButton::clicked, [this]() {
        m_result = RESTART;
        accept();
        });

    // Options — ouvre le dialog options sans fermer la pause
    connect(btnOptions, &QPushButton::clicked, [this]() {
        OptionsDialog dlg(this);
        dlg.exec();
        });

    // Controls — ouvre le dialog controls sans fermer la pause
    connect(btnControls, &QPushButton::clicked, [this]() {
        ControlsDialog dlg(this);
        dlg.exec();
        });

    // Main Menu
    connect(btnMainMenu, &QPushButton::clicked, [this]() {
        m_result = MAIN_MENU;
        accept();
        });

    layout->addWidget(btnResume);
    layout->addWidget(btnRestart);
    layout->addWidget(btnOptions);
    layout->addWidget(btnControls);
    layout->addWidget(btnMainMenu);
}

QPushButton* PauseDialog::createStyledButton(const QString& text)
{
    QPushButton* btn = new QPushButton(text);
    btn->setStyleSheet(BTN_STYLE);
    btn->setFixedHeight(55);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}
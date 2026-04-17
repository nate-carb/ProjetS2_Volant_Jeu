#include "PauseDialog.h"
#include "AnimatedButton.h"

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

PauseDialog::PauseDialog(SoundManager* soundManager, QWidget* parent)
    : QDialog(parent), m_soundManager(soundManager)
{
    setWindowTitle("En Pause");
    setFixedSize(400, 420);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background-color: transparent;");

    QWidget* card = new QWidget(this);
    card->setStyleSheet("QWidget { background-color: #4B0082; border-radius: 18px; }");

    QVBoxLayout* layout = new QVBoxLayout(card);  
    layout->setSpacing(12);
    layout->setContentsMargins(40, 30, 40, 30);

    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(card);


    // Titre PAUSED
    QLabel* title = new QLabel("En Pause");
    title->setStyleSheet(
        "color: white; font-size: 36px; font-weight: bold;"
        "background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);
    layout->addSpacing(10);

    // Boutons
    QPushButton* btnResume = createStyledButton("Reprendre");
    QPushButton* btnRestart = createStyledButton("Recommencer");
    QPushButton* btnOptions = createStyledButton("Options");
    QPushButton* btnControls = createStyledButton("Contrôles");
    QPushButton* btnMainMenu = createStyledButton("Menu Principal");
    card->setFocusPolicy(Qt::NoFocus);

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
    connect(btnOptions, &QPushButton::clicked, [this, soundManager]() {
        OptionsDialog* dlg = new OptionsDialog(soundManager, this);
        dlg->show(); 
        });

    connect(btnControls, &QPushButton::clicked, [this]() {
        ControlsDialog* dlg = new ControlsDialog(this);
        dlg->show(); 
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
    btnResume->setFocus();
}

QPushButton* PauseDialog::createStyledButton(const QString& text)
{
    AnimatedButton* btn = new AnimatedButton(text);
    btn->setFixedHeight(55);
    return btn;
}
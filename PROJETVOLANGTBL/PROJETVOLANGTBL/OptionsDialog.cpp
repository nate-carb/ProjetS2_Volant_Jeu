#include "OptionsDialog.h"
#include "AnimatedButton.h"

OptionsDialog::OptionsDialog(SoundManager* soundManager, QWidget* parent)
    : QDialog(parent), m_soundManager(soundManager)
{
    setWindowTitle("Options");
    setFixedSize(500, 250);
    setStyleSheet("background-color: #4B0082;");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(20);
    layout->setContentsMargins(40, 30, 40, 30);

    // Titre
    QLabel* title = new QLabel("Options");
    title->setStyleSheet(
        "background-color: #111111; color: white; font-size: 28px;"
        "font-weight: bold; border-radius: 8px; padding: 10px;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // Volume slider
    QHBoxLayout* volRow = new QHBoxLayout();
    QLabel* volLabel = new QLabel("Volume");
    volLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");

    QSlider* volSlider = new QSlider(Qt::Horizontal);
    volSlider->setRange(0, 100);
    int initialVolume = soundManager ? (int)(soundManager->getVolume() * 100) : 80;
    volSlider->setValue(initialVolume);
    volSlider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            background: #111111; height: 8px; border-radius: 4px;
        }
        QSlider::handle:horizontal {
            background: white; width: 20px; height: 20px;
            margin: -6px 0; border-radius: 10px;
        }
        QSlider::sub-page:horizontal {
            background: #111111; border-radius: 4px;
        }
    )");

    connect(volSlider, &QSlider::valueChanged, [this, soundManager](int val) {
        m_volume = val;
        if (soundManager)
            soundManager->setVolume(val / 100.0f);
        emit volumeChanged(val);
        });

    volRow->addWidget(volLabel);
    volRow->addWidget(volSlider);
    layout->addLayout(volRow);

    layout->addStretch();

    // Bouton fermer
    AnimatedButton* btnClose = new AnimatedButton("Fermer");
    btnClose->setFixedHeight(50);
    btnClose->setCursor(Qt::PointingHandCursor);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(btnClose);
}
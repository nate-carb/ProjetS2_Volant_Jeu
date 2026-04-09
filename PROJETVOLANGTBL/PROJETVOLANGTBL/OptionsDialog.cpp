#include "OptionsDialog.h"

OptionsDialog::OptionsDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Options");
    setFixedSize(500, 250);
    setStyleSheet("background-color: #CC2200;");

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
    volSlider->setValue(m_volume);
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

    connect(volSlider, &QSlider::valueChanged, [this](int val) {
        m_volume = val;
        });

    volRow->addWidget(volLabel);
    volRow->addWidget(volSlider);
    layout->addLayout(volRow);

    layout->addStretch();

    // Bouton fermer
    QPushButton* btnClose = new QPushButton("Close");
    btnClose->setStyleSheet(
        "QPushButton { background-color: #111111; color: white; font-size: 18px;"
        "font-weight: bold; border-radius: 8px; padding: 10px; }"
        "QPushButton:hover { background-color: #880000; }");
    btnClose->setCursor(Qt::PointingHandCursor);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(btnClose);
}
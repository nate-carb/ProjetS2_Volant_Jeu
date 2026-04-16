#include "ControlsDialog.h"
#include "AnimatedButton.h"

ControlsDialog::ControlsDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Contrôles");
    setFixedSize(900, 650);
    setStyleSheet("background-color: #4B0082;");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 20, 30, 20);

    // Titre
    QLabel* title = new QLabel("Contrôles");
    title->setStyleSheet(
        "background-color: #111111; color: white; font-size: 28px;"
        "font-weight: bold; border-radius: 8px; padding: 10px;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // Image du diagramme
    QLabel* img = new QLabel();
    QPixmap px("images/controller_diagram.png");
    if (!px.isNull()) {
        img->setPixmap(px.scaled(800, 500,
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else {
        img->setText("[ Controller Diagram ]");
        img->setStyleSheet(
            "color: white; font-size: 16px; background-color: #111111;"
            "border-radius: 8px; padding: 20px;");
    }
    img->setAlignment(Qt::AlignCenter);
    layout->addWidget(img);

    layout->addStretch();

    AnimatedButton* btnClose = new AnimatedButton("Fermer");
    btnClose->setFixedHeight(50);
    btnClose->setCursor(Qt::PointingHandCursor);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(btnClose);
}
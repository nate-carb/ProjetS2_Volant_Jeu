#include "HUDOverlay.h"
#include <QPainter>
#include <QPainterPath>
#include <QTime>

HUDOverlay::HUDOverlay(QWidget* parent) : QWidget(parent)
{
    // Transparent pour laisser voir le 3D en dessous
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
}

void HUDOverlay::updateData(float carburant, float nos, float tireWear, Vehicule::Weather weather)
{
    m_carburant = carburant;
    m_nos = nos;
    m_tireWear = tireWear;
    m_weather = weather;
    update();  // redessine
}

void HUDOverlay::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // ===== FOND HUD =====
    QPainterPath bg;
    bg.addRoundedRect(15, 15, 200, 160, 12, 12);
    painter.fillPath(bg, QColor(0, 0, 0, 150));

    // ===== BARRES =====
    // Carburant — couleur selon niveau
    QColor fuelColor;
    if (m_carburant > 50)       fuelColor = QColor(80, 220, 80);
    else if (m_carburant > 25)  fuelColor = QColor(255, 165, 0);
    else                        fuelColor = QColor(220, 50, 50);
    drawBar(painter, 25, 30, 170, 22, m_carburant, fuelColor, "Carburant");

    // NOS — toujours bleu cyan
    drawBar(painter, 25, 70, 170, 22, m_nos, QColor(0, 200, 255), "NOS");

    // Pneus — couleur selon usure
    QColor tireColor;
    if (m_tireWear > 60)        tireColor = QColor(80, 220, 80);
    else if (m_tireWear > 30)   tireColor = QColor(255, 165, 0);
    else                        tireColor = QColor(220, 50, 50);
    drawBar(painter, 25, 110, 170, 22, m_tireWear, tireColor, "Pneus");

    // ===== MÉTÉO =====
    QString weatherText;
    QColor weatherColor;
    switch (m_weather) {
    case Vehicule::SUNNY:
        weatherText = "☀ Ensoleillé";
        weatherColor = QColor(255, 220, 0);
        break;
    case Vehicule::RAINY:
        weatherText = "🌧 Pluie";
        weatherColor = QColor(100, 180, 255);
        break;
    case Vehicule::STORMY:
        weatherText = "⛈ Tempête";
        weatherColor = QColor(180, 100, 255);
        break;
    }
    painter.setPen(weatherColor);
    painter.setFont(QFont("Arial", 11, QFont::Bold));
    painter.drawText(25, 155, weatherText);

    // ===== PLUIE =====
    if (m_weather == Vehicule::RAINY || m_weather == Vehicule::STORMY) {
        int numDrops = (m_weather == Vehicule::STORMY) ? 150 : 75;
        float penWidth = (m_weather == Vehicule::STORMY) ? 2.5f : 1.5f;
        painter.setPen(QPen(QColor(150, 150, 255, 150), penWidth));
        srand(QTime::currentTime().msec());
        for (int i = 0; i < numDrops; i++) {
            int x = rand() % width();
            int y = rand() % height();
            int length = (m_weather == Vehicule::STORMY) ? 20 : 12;
            painter.drawLine(x, y, x - 3, y + length);
        }
    }
}

void HUDOverlay::drawBar(QPainter& painter, int x, int y, int w, int h,
    float value, QColor color, const QString& label)
{
    // Fond de la barre
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(60, 60, 60, 200));
    painter.drawRoundedRect(x, y, w, h, 4, 4);

    // Remplissage selon valeur
    int fillW = (int)(w * value / 100.0f);
    if (fillW > 0) {
        painter.setBrush(color);
        painter.drawRoundedRect(x, y, fillW, h, 4, 4);
    }

    // Label + valeur
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(x + 5, y + h - 5, label);

    painter.setFont(QFont("Arial", 9));
    painter.drawText(x + w - 35, y + h - 5, QString("%1%").arg((int)value));
}
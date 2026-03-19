#pragma once
#include <QWidget>
#include "Vehicule.h"

class HUDOverlay : public QWidget
{

public:
    explicit HUDOverlay(QWidget* parent = nullptr);

    void updateData(float carburant, float nos, float tireWear, Vehicule::Weather weather);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    float m_carburant = 100.0f;
    float m_nos = 100.0f;
    float m_tireWear = 100.0f;
    Vehicule::Weather m_weather = Vehicule::SUNNY;

    void drawBar(QPainter& painter, int x, int y, int w, int h,
        float value, QColor color, const QString& label);
};
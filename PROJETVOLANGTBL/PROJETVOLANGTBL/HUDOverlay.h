#pragma once
#include <QWidget>
#include <QPixmap>
#include "Vehicule.h"
#include "raceTimes.h"
#include "Track.h"
#include "PitStop.h"

class HUDOverlay : public QWidget
{
public:
    explicit HUDOverlay(QWidget* parent = nullptr);

    void updateData(float carburant, float nos, float tireWear,
        Vehicule::Weather weather,
        float speedKmh, float rpm, float maxRpm,
        int currentLap, int totalLaps,
        qint64 bestLapMs, qint64 currentLapMs, qint64 deltaMs,
        bool warning, const RaceTimes* raceTimes,
        const Track* track, const PitStop* pitStop,        
        QVector2D carPos, float carAngle);     

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    // Données
    float  m_carburant = 100.0f;
    float  m_nos = 100.0f;
    float  m_tireWear = 100.0f;
    Vehicule::Weather m_weather = Vehicule::SUNNY;
    float  m_speedKmh = 0.0f;
    float  m_rpm = 800.0f;
    float  m_maxRpm = 8000.0f;
    int    m_currentLap = 1;
    int    m_totalLaps = 3;
    qint64 m_bestLapMs = -1;
    qint64 m_currentLapMs = 0;
    qint64 m_deltaMs = 0;
    bool   m_warning = false;
    const RaceTimes* m_raceTimes = nullptr;
    bool m_paused = false;

    // Images
    QPixmap m_pixLap;
    QPixmap m_pixLapTime;
    QPixmap m_pixSpeedRpm;
    QPixmap m_pixWarning;
    QPixmap m_pixBoostEmpty;

    // Helpers
    void drawBar(QPainter& p, int x, int y, int w, int h,
        float value, QColor color, const QString& label);
    //void drawBoostBar(QPainter& p, int x, int y, float nosRatio);
    void drawLapCounter(QPainter& p);
    void drawLapTimes(QPainter& p);
    void drawSpeedRpm(QPainter& p);
    void drawWarning(QPainter& p);
    void drawBoostBar(QPainter& p);

    const Track* m_track = nullptr;
    const PitStop* m_pitStop = nullptr;
    QVector2D m_carPos;
    float     m_carAngle = 0.0f;

    // Déclare la fonction
    void drawMinimap(QPainter& p);

};
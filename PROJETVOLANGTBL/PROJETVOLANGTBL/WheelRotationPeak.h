#pragma once
#ifndef WHEELROTATIONPEAK_H
#define WHEELROTATIONPEAK_H

#include <QObject>
#include <cmath>

class WheelRotationPeak : public QObject
{
    Q_OBJECT

public:
    struct Sample {
        float rotMag   = 0.0f;   // magnitude courante dans le plan de rotation
        float rotAccel = 0.0f;   // variation de magnitude depuis le dernier update
    };

    explicit WheelRotationPeak(QObject* parent = nullptr);

    // Appeler à chaque frame avec les valeurs brutes de l'accéléromètre
    // et le deltaTime en secondes.
    void update(float accelX, float accelY, float accelZ, float deltaTime);

    // Résultats
    float  getPeakRotMag()   const { return m_peakRotMag; }
    float  getPeakRotAccel() const { return m_peakRotAccel; }
    Sample getCurrent()      const { return m_current; }

    // Remet les pics à zéro
    void resetPeaks();

signals:
    // Émis seulement quand un nouveau pic d'accélération angulaire est battu
    void newPeakRotAccel(float peak);
    // Émis seulement quand un nouveau pic de vitesse angulaire est battu
    void newPeakRotMag(float peak);

private:
    float  m_prevMag      = -1.0f;   // -1 = pas encore initialisé
    float  m_peakRotMag   = 0.0f;
    float  m_peakRotAccel = 0.0f;
    Sample m_current;
};

#endif 

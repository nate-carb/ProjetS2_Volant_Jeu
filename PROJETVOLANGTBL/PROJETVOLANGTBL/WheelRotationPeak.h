#pragma once
#ifndef WHEELROTATIONPEAK_H
#define WHEELROTATIONPEAK_H

#include <QObject>
#include <cmath>

// ---------------------------------------------------------------------------
//  WheelRotationPeak
//
//  Mesure la plus grosse accélération de rotation du volant à partir
//  des valeurs brutes X,Y,Z d'un accéléromètre placé sur la jante.
//
//  Principe physique :
//    - Le volant tourne autour de son axe de colonne (axe Z ici).
//    - L'accéléromètre en jante ressent :
//        * Accélération centripète   : ac = ω² · r  (vers le centre)
//        * Accélération tangentielle : at = α · r    (dans le plan, perp.)
//    - La magnitude dans le plan XY  = sqrt(X²+Y²)
//      est proportionnelle à la vitesse angulaire ω.
//    - Le taux de variation de cette magnitude / dt
//      est proportionnel à l'accélération angulaire α.
//
//  On suit :
//    • peakRotMag   : pic de sqrt(X²+Y²)         → pic de vitesse angulaire
//    • peakRotAccel : pic du |Δmag/Δt|            → pic d'accélération angulaire
//
//  Note : les valeurs sont en unités de l'accéléromètre (ex: m/s² ou g).
//  Pour convertir en rad/s ou rad/s², divise par le rayon r (en mètres).
// ---------------------------------------------------------------------------
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

    // Remet les pics à zéro (ex: nouveau tour / nouvelle course)
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

#endif // WHEELROTATIONPEAK_H

#include "WheelRotationPeak.h"
#include <QDebug>

WheelRotationPeak::WheelRotationPeak(QObject* parent)
    : QObject(parent)
{
}

void WheelRotationPeak::update(float accelX, float accelY, float accelZ, float deltaTime)
{
    // ── 1. Magnitude dans le plan de rotation (XY) ───────────────────────────
    //    Z est l'axe de la colonne du volant → on l'ignore ici.
    //    Si ton accéléromètre est monté différemment, adapte les axes.
    float rotMag = std::sqrt(accelX * accelX + accelY * accelY);

    // ── 2. Variation de magnitude = proxy accélération angulaire ─────────────
    float rotAccel = 0.0f;
    if (m_prevMag >= 0.0f && deltaTime > 0.0f) {
        rotAccel = std::abs(rotMag - m_prevMag) / deltaTime;
    }
    m_prevMag = rotMag;

    // ── 3. Mise à jour du sample courant ─────────────────────────────────────
    m_current.rotMag   = rotMag;
    m_current.rotAccel = rotAccel;

    // ── 4. Mise à jour des pics ───────────────────────────────────────────────
    if (rotMag > m_peakRotMag) {
        m_peakRotMag = rotMag;
        qDebug() << "[WheelRotationPeak] Nouveau pic vitesse angulaire :"
                 << m_peakRotMag;
        emit newPeakRotMag(m_peakRotMag);
    }

    if (rotAccel > m_peakRotAccel) {
        m_peakRotAccel = rotAccel;
        qDebug() << "[WheelRotationPeak] Nouveau pic accélération angulaire :"
                 << m_peakRotAccel;
        emit newPeakRotAccel(m_peakRotAccel);
    }
}

void WheelRotationPeak::resetPeaks()
{
    m_peakRotMag   = 0.0f;
    m_peakRotAccel = 0.0f;
    m_prevMag      = -1.0f;
    m_current      = {};
    qDebug() << "[WheelRotationPeak] Pics remis à zéro.";
}

#include "RaceStart.h"

RaceStart::RaceStart(QObject* parent) : QObject(parent)
{
    m_lightTimer = new QTimer(this);
}

void RaceStart::reset()
{
    m_lightTimer->stop();
    m_state = IDLE;
    m_lightsOn = 0;
    m_penaltyActive = false;
    m_penaltyTimeLeft = 0.0f;
}

void RaceStart::startSequence()
{
    m_state = LIGHTS_ON;
    m_lightsOn = 0;
    m_penaltyActive = false;

    // Allume une lumière toutes les 800ms
    m_lightTimer->disconnect();
    connect(m_lightTimer, &QTimer::timeout, this, [this]() {
        m_lightsOn++;
        if (m_lightsOn >= 5) {
            m_lightTimer->stop();
            // Attend un vrai muon (ou timeout failsafe)
            m_waitTimer.start();
            m_state = LIGHTS_OFF;
        }
        });
    m_lightTimer->start(800);
}

void RaceStart::muonDetected()
{
    // Un muon a été détecté par l'Arduino → éteindre les feux
    if (m_state == LIGHTS_OFF) {
        triggerLightsOut();
    }
}

void RaceStart::triggerLightsOut()
{
    m_state = RACING;
    m_lightsOn = 0;
    emit raceStarted();
}

void RaceStart::playerAccelerated()
{
    if (m_state == LIGHTS_ON || m_state == LIGHTS_OFF) {
        m_state = FALSE_START;
        m_penaltyActive = true;
        m_penaltyTimeLeft = PENALTY_DURATION;
        m_lightTimer->stop();
        emit falseStartDetected();
        emit raceStarted();
    }
}

void RaceStart::update(float deltaTime)
{
    // Failsafe : si aucun muon après TIMEOUT_MS, on démarre quand même
    if (m_state == LIGHTS_OFF && m_waitTimer.elapsed() >= TIMEOUT_MS) {
        triggerLightsOut();
        return;
    }

    // Décompte pénalité faux départ
    if (m_penaltyActive) {
        m_penaltyTimeLeft -= deltaTime;
        if (m_penaltyTimeLeft <= 0.0f) {
            m_penaltyTimeLeft = 0.0f;
            m_penaltyActive = false;
            m_state = RACING;
            
        }
    }
}
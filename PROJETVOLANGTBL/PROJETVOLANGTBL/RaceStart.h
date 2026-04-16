#pragma once
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

class RaceStart : public QObject
{
    Q_OBJECT
public:
    enum State { IDLE, LIGHTS_ON, LIGHTS_OFF, RACING, FALSE_START };

    explicit RaceStart(QObject* parent = nullptr);

    void startSequence();
    void reset();
    void playerAccelerated();
    void muonDetected();           //appelle quand l'Arduino détecte un muon

    bool  isRacing()          const { return m_state == RACING; }
    bool  isFalseStart()      const { return m_state == FALSE_START; }
    bool  isPenalty()         const { return m_penaltyActive; }
    int   getLightsOn()       const { return m_lightsOn; }
    State getState()          const { return m_state; }
    float getPenaltyTimeLeft() const { return m_penaltyTimeLeft; }
    void  update(float deltaTime);

signals:
    void raceStarted();
    void falseStartDetected();

private:
    void triggerLightsOut();      // logique commune d'extinction des feux

    State         m_state = IDLE;
    int           m_lightsOn = 0;
    QTimer* m_lightTimer = nullptr;
    QElapsedTimer m_waitTimer;           // mesure le temps d'attente en LIGHTS_OFF
    bool          m_penaltyActive = false;
    float         m_penaltyTimeLeft = 0.0f;
    const float   PENALTY_DURATION = 5.0f;
    const int     TIMEOUT_MS = 10000; // failsafe si aucun muon en 10s
};

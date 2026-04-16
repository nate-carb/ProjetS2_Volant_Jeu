#pragma once
#include <QElapsedTimer>
#include <vector>
#include <QString>
#include "track.h"

struct CheckpointStamp {
    int id;
    qint64 elapsedMs;
};

class RaceTimes
{
public:
    void setupRace(int nlaps, Track* track);
    void startRace();
    void finishRace();
    void checkForCheckpoint(Track* track, QVector2D pos);
    bool isRaceStarted() { return raceStarted; }
    bool isRaceFinished() const { return raceFinished; }
    void resetRace() { raceFinished = false; raceStarted = false; }
    void forceFinish() { raceFinished = true; raceStarted = false; }

    QString getLastCheckpointTimeString();

    // ── Nouveaux getters ──────────────────────────────
    int  getCurrentLap()   const;
    int  getTotalLaps()    const { return numOflaps; }
    qint64 getCurrentLapMs() const;   // temps du lap en cours
    qint64 getBestLapMs()    const;   // meilleur lap terminé (-1 si aucun)
    qint64 getDeltaMs()      const;   // écart vs best (-1 si indisponible)
    qint64 getTotalRaceTimeMs() const {
        if (stamps.empty()) return 0;
        return stamps.back().elapsedMs;
    }

    QString formatMs(qint64 ms) const; // "1:23.456"

private:
    QElapsedTimer elapsedTimer;
    std::vector<CheckpointStamp> stamps;
    int lastCheckpointId = 0;
    bool raceStarted = false;
    int numOflaps = 1;
    int checkpointsPerLap = 1;
    int checkpointCounter = 0;
    bool raceFinished = false;
};
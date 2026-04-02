#include "raceTimes.h"
#include <QDebug>
#include <algorithm>

void RaceTimes::setupRace(int nlaps, Track* track)
{
    numOflaps = nlaps;
    checkpointsPerLap = track->getCheckpoints().size();
}

void RaceTimes::startRace()
{
    stamps.clear();
    elapsedTimer.start();
    stamps.push_back({ 0, 0 });
    lastCheckpointId = 0;
    checkpointCounter = 1;
    raceStarted = true;
}

void RaceTimes::finishRace()
{
    if (checkpointCounter < (numOflaps * checkpointsPerLap))
        checkpointCounter++;
    else {
        qDebug() << "Race finish";
        raceStarted = false;
    }
}

void RaceTimes::checkForCheckpoint(Track* track, QVector2D position)
{
    int carOnCheckpoint = track->isCarBetweenCheckpoints(position);
    if ((carOnCheckpoint > -1) && (lastCheckpointId != carOnCheckpoint)) {
        lastCheckpointId = carOnCheckpoint;
        qint64 elapsed = elapsedTimer.elapsed();
        stamps.push_back({ carOnCheckpoint, elapsed });
        qDebug() << "Checkpoint #:" << carOnCheckpoint << "\tElapsed:" << elapsed;
        finishRace();
    }
}

// ── Lap actuel (1-based) ──────────────────────────────────────────────────
int RaceTimes::getCurrentLap() const
{
    if (!raceStarted || stamps.empty()) return 1;
    int completedCheckpoints = (int)stamps.size() - 1; // -1 pour le stamp de départ
    return completedCheckpoints / checkpointsPerLap + 1;
}

// ── Temps du lap en cours (ms depuis début du lap) ───────────────────────
qint64 RaceTimes::getCurrentLapMs() const
{
    if (!raceStarted) return 0;
    int currentLap = getCurrentLap();
    int lapStartIndex = (currentLap - 1) * checkpointsPerLap; // index dans stamps
    qint64 lapStartMs = (lapStartIndex < (int)stamps.size())
        ? stamps[lapStartIndex].elapsedMs : 0;
    return elapsedTimer.elapsed() - lapStartMs;
}

// ── Meilleur lap terminé ─────────────────────────────────────────────────
qint64 RaceTimes::getBestLapMs() const
{
    qint64 best = -1;
    for (int lap = 1; lap <= numOflaps; lap++) {
        int endIdx = lap * checkpointsPerLap;
        int startIdx = (lap - 1) * checkpointsPerLap;
        if (endIdx >= (int)stamps.size()) break; // lap pas encore terminé
        qint64 lapTime = stamps[endIdx].elapsedMs - stamps[startIdx].elapsedMs;
        if (best < 0 || lapTime < best) best = lapTime;
    }
    return best;
}

// ── Delta : écart avec le best au même checkpoint ────────────────────────
qint64 RaceTimes::getDeltaMs() const
{
    qint64 best = getBestLapMs();
    if (best < 0) return 0;
    return getCurrentLapMs() - best;
}

// ── Format "M:SS.mmm" ────────────────────────────────────────────────────
QString RaceTimes::formatMs(qint64 ms) const
{
    if (ms < 0) return "--:--.---";
    qint64 absMs = std::abs(ms);
    int minutes = (int)(absMs / 60000);
    int seconds = (int)((absMs % 60000) / 1000);
    int millis = (int)(absMs % 1000);
    return QString("%1:%2.%3")
        .arg(minutes)
        .arg(seconds, 2, 10, QChar('0'))
        .arg(millis, 3, 10, QChar('0'));
}

QString RaceTimes::getLastCheckpointTimeString()
{
    if (stamps.size() < 2) return "--:--.---";
    return formatMs(stamps.back().elapsedMs);
}
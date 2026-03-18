#include "raceTimes.h"
#include <QDebug>

void RaceTimes::startRace()
{
	stamps.clear(); // Clear all the times
	elapsedTimer.start();
	stamps.push_back({0, 0}); // Checkpoint id : 0 (Startline), time : 0
	lastCheckpointId = 0;
	raceStarted = true;
}

void RaceTimes::finishRace()
{

}

void RaceTimes::checkForCheckpoint(Track* track, QVector2D position)
{
	int carOnCheckpoint = track->isCarBetweenCheckpoints(position);
	if ((carOnCheckpoint > -1) && (lastCheckpointId != carOnCheckpoint)) {
		
		lastCheckpointId = carOnCheckpoint; //Update lastCheckpoint

		qint64 elapsed = elapsedTimer.elapsed(); // Get the time

		stamps.push_back({ carOnCheckpoint, elapsed }); // ADD to the list
		qDebug() << "Checkpoints #:" << carOnCheckpoint << "\t Elapsed Time " << elapsed;
	}

}



#include "raceTimes.h"
#include <QDebug>

void RaceTimes::setupRace(int nlaps, Track* track)
{
	numOflaps = nlaps;
	checkpointsPerLap = track->getCheckpoints().size();
	//qDebug() << "CPL" << checkpointsPerLap;
}

void RaceTimes::startRace()
{
	stamps.clear(); // Clear all the times
	elapsedTimer.start();
	stamps.push_back({0, 0}); // Checkpoint id : 0 (Startline), time : 0
	lastCheckpointId = 0;
	checkpointCounter = 1;
	raceStarted = true;
}

void RaceTimes::finishRace()
{
	if (checkpointCounter < (numOflaps * checkpointsPerLap)) {
		checkpointCounter ++;
		
	}
	else {
		qDebug() << "Race finish";
		raceStarted = false;
	}
	
}

void RaceTimes::checkForCheckpoint(Track* track, QVector2D position)
{
	int carOnCheckpoint = track->isCarBetweenCheckpoints(position);
	if ((carOnCheckpoint > -1) && (lastCheckpointId != carOnCheckpoint)) {
		
		lastCheckpointId = carOnCheckpoint; //Update lastCheckpoint

		qint64 elapsed = elapsedTimer.elapsed(); // Get the time

		stamps.push_back({ carOnCheckpoint, elapsed }); // ADD to the list
		qDebug() << "Checkpoints #:" << carOnCheckpoint << "\t Elapsed Time " << elapsed;

		finishRace();
	}

}



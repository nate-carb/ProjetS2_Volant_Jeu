#pragma once
#include <QElapsedTimer>
#include <vector>
#include "track.h"

struct CheckpointStamp {
	int id;
	qint64 elapsedMs;
};

class RaceTimes
{
	public:
		void setupRace(int nlaps, Track* track); //nlaps : number of laps for the race , track : the current track of the race
		void startRace();
		void finishRace();
		void checkForCheckpoint(Track* track, QVector2D pos); // ptr Track, position of the voiture
		bool isRaceStarted() { return raceStarted; };

	private:
		QElapsedTimer elapsedTimer;
		std::vector<CheckpointStamp> stamps;
		int lastCheckpointId;
		bool raceStarted = false;

		// RACE PARAMS
		int numOflaps = 1;
		int checkpointsPerLap;
		int checkpointCounter;

};


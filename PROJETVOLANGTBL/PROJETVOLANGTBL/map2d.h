#ifndef MAP2D_H
#define MAP2D_H

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include "Track.h"


constexpr int GRID_W = 80;
constexpr int GRID_H = 30; 

class Map2D : public Track

{
	public:
		Map2D();
		~Map2D();

		void displayTrack();
		void drawMapASCII() const;

	private:
		void computeBounds(float& minX, float& maxX, float& minY, float& maxY);
		void drawTrackASCII();
};




#endif	// MAP2D_H

#ifndef MAP2D_H
#define MAP2D_H

#include <iostream>
#include <vector>
#include <cmath>
#include <string>


struct Vec2 {
    float x;
    float y;

    Vec2() : x(0), y(0) {}
    Vec2(float _x, float _y) : x(_x), y(_y) {}

    Vec2 move(float angleDeg, float distance) const {
        float rad = angleDeg * (3.14159265f / 180.0f);
        return Vec2(x + distance * cos(rad),
            y + distance * sin(rad));
    }

    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }

    float length() const { return std::sqrt(x * x + y * y); }

    Vec2 normalized() const {
        float len = length();
        return (len > 0) ? Vec2(x / len, y / len) : Vec2(0, 0);
    }

    Vec2 perpendicular() const { return Vec2(-y, x); } // rotate 90°
};

struct TrackEdges {
    std::vector<Vec2> left;
    std::vector<Vec2> right;
};

struct trackInfo {

    Vec2 start;
    float startAngle;
    std::vector<Vec2> centerLine;
    TrackEdges trackEdges;

};


constexpr int GRID_W = 80;
constexpr int GRID_H = 30; 

class Map2D 

{
	public:
		Map2D();
		~Map2D();
		void setChoixMap(int x);
		int getChoixMap();

		//std::vector<trackInfo> getTrackList()
		void trackGenerator();
		Vec2 getStartPos();

        std::vector<trackInfo> getTrackList();
        void drawMapASCII() const;

	private:
		int choixMap = 0;
		void setDefaultTrack();
       /* TrackEdges calculateTrackEdges(
            const std::vector<Vec2>& centerLine,
            float trackWidth);*/
		//track generator infos 
		std::vector<trackInfo> trackList;
        int trackListTaille;
        void calculAngLen(
            std::vector<Vec2>& track,
            float& currentAngle, Vec2& currentPos,
            std::vector<float>& angles, std::vector<float>& lengths
        );
		void addStartPos(Vec2 pos);
		void addStartAng(float ang);
        
        
		
		
		
		
};




#endif	// MAP2D_H

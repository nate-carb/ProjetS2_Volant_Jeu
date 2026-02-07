#pragma once
#ifndef TRACK_H
#define TRACK_H
#include <vector>
#include <iostream>

#define VIRAGE_45RIGHT 1
#define VIRAGE_45LEFT 2
#define VIRAGE_90RIGHT 3
#define VIRAGE_90LEFT 4
#define STRAIGHT 5
#define STARTLINE 6
#define GARAGE 7
#define PIT 8
#define GRANDSTAND 9
#define BRIDGES 10
// * ----------------------------------------------------------
// *  Stuctures 
// *-----------------------------------------------------------

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


// * ----------------------------------------------------------
// *  Classes
// *-----------------------------------------------------------

class TrackPieces 
{
	public:
		virtual ~TrackPieces();
		int getPos() { return pos; };
		void setPos(int x) { pos = x; };
		int getId() { return id; };
		std::vector<float> getAngles() { return angles; };
		std::vector<float> getLengths() { return lengths; };

	private :
		int pos;

	protected:
		int id;
		std::vector<float> angles;
		std::vector<float> lengths;
		
};

class Virage_45right : public TrackPieces
{
	Virage_45right();
	
};

class Virage_45left : public TrackPieces
{
	Virage_45left();
};

class Virage_90right : public TrackPieces
{
	Virage_90right();
};

class Virage_90left : public TrackPieces
{
	Virage_90left();
};

class Straight : public TrackPieces
{
	Straight();
};

class StartLine : public TrackPieces
{
	StartLine();
};

class Garage : public TrackPieces
{
	Garage();
};

class Pit : public TrackPieces
{
	Pit();
};

class Grandstand : public TrackPieces
{
	Grandstand();
};

class Bridges : public TrackPieces
{
	Bridges();

};


class Track
{
public:
	Track();
	Track(std::vector<int> listPieces);
	//virtual ~Track();
	//virtual void generateTrack() = 0;
	void calculateTrackEdges();
	void calculAngLen(int index);
	//virtual void displayTrack() const = 0;

	
	std::vector<TrackPieces> getPieces() const { return pieces; };
	std::vector<Vec2> getCenterLine() const { return centerLine; };
	TrackEdges getTrackEdges() const { return trackEdges; };

private:
	std::vector<TrackPieces> pieces;
	std::vector<Vec2> centerLine;
	TrackEdges trackEdges;

	float startAngle;
	float currentAngle;
	Vec2 currentPos;
	float trackWidth;
	std::vector<int> piecesIntList;
	

};

#endif	
#pragma once
#ifndef TRACK_H
#define TRACK_H
#include <vector>
#include <iostream>
#include <qvector2d.h>

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

struct TrackEdges {
	std::vector<QVector2D> left;
	std::vector<QVector2D> right;
};

struct PitLane {
	std::vector<QVector2D> centerLine;
	TrackEdges edges;
	QVector2D entryPoint;  // where pit splits from main track
	QVector2D exitPoint;   // where pit rejoins main track
	std::vector<QVector2D> entryCurve;
	TrackEdges entryCurveEdges;
	std::vector<QVector2D> exitCurve;
	TrackEdges exitCurveEdges;
	bool isValid = false;
};

QVector2D perpendicular(QVector2D v);

QVector2D move(QVector2D v, float angleDeg, float distance);

float distancePointToSegment(const QVector2D& P, const QVector2D& A, const QVector2D& B);

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
		QString getSpritePath() { return spritePath; };
		float getSpriteRotationOffset() { return spriteRotationOffset; };

		float getTrackWidth() { return trackWidth; };
		float getTurnRadius() { return turnRadius; };
		std::vector<float> getAngles() { return angles; };
		std::vector<float> getLengths() { return lengths; };

		void setStartIndex(int index) { startIndex = index; };
		 int getStartIndex() { return startIndex; };
		 void setEndIndex(int index) { endIndex = index; };
		 int getEndIndex() { return endIndex; };

	private:
		float trackWidth = 40.0f;
		float turnRadius = trackWidth / 2.0f; //trackWidth / 2.0f;

	protected:
		QString spritePath;
		float spriteRotationOffset = 0.0f;
		int pos;
		int id;
		int startIndex = -1;
		int endIndex = -1;
		float stepAngleDeg;
		std::vector<float> angles;
		std::vector<float> lengths;
		//float lengthAngleVirage = trackWidth/8 / cos(qDegreesToRadians(45.0f / 4));
		
};

class Virage_45right : public TrackPieces
{
	public:
		Virage_45right();
	
};

class Virage_45left : public TrackPieces
{
	public:
		Virage_45left();
};

class Virage_90right : public TrackPieces
{
	public:
		Virage_90right();
};

class Virage_90left : public TrackPieces
{
	public:
		Virage_90left();
};

class Straight : public TrackPieces
{
	public:
		Straight();
};

class StartLine : public TrackPieces
{
	public:
		StartLine();
};


class Pit : public TrackPieces
{
	public:
		Pit();
		
};



struct DecorsInfo {
	float angle;
	QVector2D pos;
	float scale = 1.0f;
	float width;
	float length;
	QString modelPath;

};

class DecorPieces 
{
	public:
		DecorPieces();
		virtual ~DecorPieces();
		QVector2D getPos() { return info.pos; };
		void setPos(QVector2D p) { info.pos = p; };
		float getAngle() { return info.angle; };
		void setAngle(float a) { info.angle = a; };
		float getWidth() { return info.width; };
		float getLength() { return info.length; };

		// For model loading
		QString getModelPath() { return info.modelPath; };
		
		float getScale() { return info.scale; };
		void setScale(float s) { info.scale = s; };
		//void setModelPath(QString path) { modelPath = path; };

		virtual void selectModel(int modelNum) = 0; // Pure virtual function to select a model based on some criteria 

protected:
	DecorsInfo info;
	std::vector<QString> modelList; 
};

class Grandstand : public DecorPieces
{
	public:
		Grandstand(QVector2D positon, float angle);

		void selectModel(int modelNum);
		
};

class Garage : public DecorPieces
{
	public:
		Garage(QVector2D positon, float angle);
		void selectModel(int modelNum);
};

class TREES : public DecorPieces
{
	public:
		TREES(QVector2D positon, float angle);
		void randomModel(); // Assign a random tree model from a predefined set

		void selectModel(int modelNum);
	
};

class NoSpecificDecor : public DecorPieces
{
	public:
		NoSpecificDecor(QVector2D positon, float angle);
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
	
	// Save/Load functions
	bool saveToFile(const std::string& filename) const;
	bool loadFromFile(const std::string& filename);
	
	bool isVector2DOnTrack(const QVector2D& point) const;
	float getTrackWidth() const { return trackWidth; };
	std::vector<TrackPieces*> getPieces() const { return pieces; };
	std::vector<QVector2D> getCenterLine() const { return centerLine; };
	TrackEdges getTrackEdges() const { return trackEdges; };
    std::vector<int> getPiecesList() const { return piecesIntList; }

	// Pit lane related functions
	void generatePitLane(int startIndex, int endIndex);
	PitLane getPitLane() const { return pitLane; }
	bool hasPitLane() const { return pitLane.isValid; }
	int getPitStartIndex() const { return pitStartIndex; }
	int getPitEndIndex() const { return pitEndIndex; }

	// decor related functions
	std::vector<DecorPieces*> getDecors() const { return decors; };
	void addDecor(DecorPieces* decor) { decors.push_back(decor); };

private:
	std::vector<TrackPieces*> pieces;
	std::vector<QVector2D> centerLine;
	TrackEdges trackEdges;

	float startAngle;
	float currentAngle;
	QVector2D currentPos;
	float trackWidth;
	
	std::vector<int> piecesIntList;

	//Pit lane related members
	PitLane pitLane;
	int pitStartIndex = -1;  // index in centerLine where pit begins
	int pitEndIndex = -1;  // index in centerLine where pit ends

	// decor pieces
	std::vector<DecorPieces*> decors;
	

};

#endif	
#pragma once
#ifndef TRACK_H
#define TRACK_H
#include <vector>
#include <iostream>
#include <qvector2d.h>

//Track.h - Defines the Track class and related structures for representing a racing track, its pieces, and decor elements.
#define VIRAGE_45RIGHT 1
#define VIRAGE_45LEFT 2
#define VIRAGE_90RIGHT 3
#define VIRAGE_90LEFT 4
#define STRAIGHT 5
#define STARTLINE 6
//#define GARAGE 7
#define PIT 8
//#define GRANDSTAND 9
//#define BRIDGES 10
// DECORS IDs
#define GARAGE_INDEX 1
#define GRANDSTAND_INDEX 2
#define TREES_INDEX 3
#define NOSPECIFICDECOR_INDEX 4
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
// For Bezier curve calculations (WALLS)
struct BezierCurveData {
	QVector2D p0;
	QVector2D p1;
	QVector2D p2;
	QVector2D p3;
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
	float scale;
	float width; //in 2D and 3D x direction
	float height; //not use in 2D, in 3D y direction
	float length; //in 2D y direction, in 3D z direction
	QString modelPath;
	int modelType; // e.g., GARAGE, GRANDSTAND, TREES, NOSPECIFICDECOR
	int modelIndex; // index in the model list for this decor type


};

class DecorPieces 
{
	public:
		DecorPieces();
		virtual ~DecorPieces();
		DecorsInfo getInfo() { return info; };
		void setPos(QVector2D p) { info.pos = p; }
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
		void selectModel(int modelNum);
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
	void addDecor(int decorType, int decorIndexList); // decorType is the type of decor (e.g., GARAGE_INDEX, GRANDSTAND_INDEX, TREES_INDEX), decorIndexList is the index in the corresponding model list for that decor type
	void addDecorDirect(DecorPieces* d) { if (d) decors.push_back(d); }

	// Curves walls related functions
	void addBezierCurve(BezierCurveData c) { bezierCurves.push_back(c); }
	const std::vector<BezierCurveData>& getBezierCurves() const { return bezierCurves; }
	void clearBezierCurves() { bezierCurves.clear(); }
	BezierCurveData& getBezierCurveRef(int index) { return bezierCurves[index]; }
	bool hasBezierCurves() const { return !bezierCurves.empty(); }

private:
	std::vector<TrackPieces*> pieces;
	std::vector<QVector2D> centerLine;
	TrackEdges trackEdges;
	std::vector<BezierCurveData> bezierCurves; // Store Bezier curve data for walls

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
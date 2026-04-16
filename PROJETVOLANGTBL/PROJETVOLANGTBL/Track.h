#pragma once
#ifndef TRACK_H
#define TRACK_H
#include <vector>
#include <iostream>
#include <qvector2d.h>
#include <QtMath>
#include <QColor>

#define VIRAGE_45RIGHT 1
#define VIRAGE_45LEFT 2
#define VIRAGE_90RIGHT 3
#define VIRAGE_90LEFT 4
#define STRAIGHT 5
#define STARTLINE 6
#define PIT 8
#define GARAGE_INDEX 1
#define GRANDSTAND_INDEX 2
#define TREES_INDEX 3
#define NOSPECIFICDECOR_INDEX 4

struct TrackEdges {
	std::vector<QVector2D> left;
	std::vector<QVector2D> right;
};
struct CheckpointData {
	QVector2D left;  
	QVector2D right;  
	QVector2D forward; 
	int centerLineIndex = -1;
	bool triggered = false;
};

struct PitLane {
	std::vector<QVector2D> centerLine;
	TrackEdges edges;
	QVector2D entryPoint;  
	QVector2D exitPoint;   
	std::vector<QVector2D> entryCurve;
	TrackEdges entryCurveEdges;
	std::vector<QVector2D> exitCurve;
	TrackEdges exitCurveEdges;
	bool isValid = false;
};

struct BezierCurveData {
	QVector2D p0;
	QVector2D p1;
	QVector2D p2;
	QVector2D p3;
};

struct GroundRenderingData {
	float width;
	float height;
	QString texturePath;
	QColor ambientColor;
};

struct KerbRenderingData {
	float width;
	float height;
	QColor color1;
	QColor color2;
};

struct PitRenderingData {
	QColor pitColor;
	KerbRenderingData kerbData;
	QString pitTexturePath;
	QColor ambientColor;
};

struct TrackRenderingData {
	QColor trackColor;
	KerbRenderingData kerbData;
	QString trackTexturePath;
	QColor ambientColor;
};

struct ChoixMapData {
	int mapIndex;
	QString mapName;
	QString mapFilePath;
	QString mapThumbnailPath;

	QString skyboxFilePath;
	
	GroundRenderingData groundData;
	TrackRenderingData trackData;
	PitRenderingData pitData;
	
};


enum class TrackSegmentType { CURVE_TRACK, STRAIGHT_TRACK, PIT_TRACK };


struct TrackSegment {
	TrackSegmentType type;

	
	QVector2D start;
	QVector2D end;

	
	QVector2D cp1;
	QVector2D cp2;

};

QVector2D perpendicular(QVector2D v);

QVector2D move(QVector2D v, float angleDeg, float distance);

float distancePointToSegment(const QVector2D& P, const QVector2D& A, const QVector2D& B);

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
		float turnRadius = trackWidth / 2.0f; 

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
	float width; 
	float height; 
	float length; 
	QString modelPath;
	int modelType; 
	int modelIndex; 

	int  segmentIndex = -1; 
	bool autoPlaced = false;
};


class DecorPieces 
{
	public:
		DecorPieces();
		virtual ~DecorPieces();
		DecorsInfo getInfo() { return info; };
		DecorsInfo& getInfoRef() { return info; }
		void setPos(QVector2D p) { info.pos = p; }
		
		QString getModelPath() { return info.modelPath; };
		
		float getScale() { return info.scale; };
		void setScale(float s) { info.scale = s; };

		void setAngle(float a) { info.angle = qDegreesToRadians(a); };
		
		void setAutoPlaced(bool b) { info.autoPlaced = b; }
		void setSegmentIndex(int i) { info.segmentIndex = i; }

		virtual void selectModel(int modelNum) = 0; 

protected:
	DecorsInfo info;
	std::vector<QString> modelList; 
};

class CheckpointDecor : public DecorPieces
{
	public:
		CheckpointDecor(QVector2D positon, float angle);
		void selectModel(int modelNum);
	private:
		int centerlineIndex; 
		int leftEdgeIndex; 
		int rightEdgeIndex; 

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
		void randomModel(); 

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
	void calculateTrackEdges();
	void calculAngLen(int index);

	bool saveToFile(const std::string& filename) const;
	bool loadFromFile(const std::string& filename);
	bool playTrack(QString name);
	
	bool isVector2DOnTrack(const QVector2D& point) const;

	bool isVector2DOnPitLane(const QVector2D& point, bool onTrack) const;

	float getStartAngle() const { return startAngle; };

	float getTrackWidth() const { return trackWidth; };
	void setTrackWidth(float w) { trackWidth = w; }
	std::vector<TrackPieces*> getPieces() const { return pieces; };
	std::vector<QVector2D> getCenterLine() const { return centerLine; };
	TrackEdges getTrackEdges() const { return trackEdges; };
    std::vector<int> getPiecesList() const { return piecesIntList; }


	void generatePitLane(int startIndex, int endIndex);

	PitLane getPitLane() const { return pitLane; }
	bool hasPitLane() const { return pitLane.isValid; }
	int getPitStartIndex() const { return pitStartIndex; }
	int getPitEndIndex() const { return pitEndIndex; }
	void setPitStartIndex(int index) { pitStartIndex = index; }
	void setPitEndIndex(int index) { pitEndIndex = index; }

	
	std::vector<DecorPieces*> getDecors() const { return decors; };
	void addDecor(int decorType, int decorIndexList);
	void addDecorDirect(DecorPieces* d) { if (d) decors.push_back(d); }

	void autoPlaceDecorsForSegment(int segmentIndex);
	void removeAutoDecorsForSegment(int segmentIndex);
	void autoPlaceAllDecors();
	void removeAllAutoDecors();
	
	void addBezierCurve(BezierCurveData c) { bezierCurves.push_back(c); }
	const std::vector<BezierCurveData>& getBezierCurves() const { return bezierCurves; }
	void clearBezierCurves() { bezierCurves.clear(); }
	BezierCurveData& getBezierCurveRef(int index) { return bezierCurves[index]; }
	bool hasBezierCurves() const { return !bezierCurves.empty(); }

	void closeTrack(); 
	bool isClosed() const;
	float getClosureGap() const;

	void addCurveSegment();
	void addStraightSegment();
	void removeLastSegment();

	void addPitSegment();
	void buildFromSegments();
	void addTrackSegment(TrackSegment s) { trackSegments.push_back(s); }

	ChoixMapData getCurrentChoixMapData() { return currentChoixMapData; }
	void setCurrentChoixMapData( QString mapName);
	void addChoixMap(const ChoixMapData& data) { choixMapList.push_back(data); }

	void defaultMapList(); 
	
	void createCheckpointAtSegment(); 
	const std::vector<CheckpointData>& getCheckpoints() const { return checkpoints; }
	std::vector<CheckpointData>& getCheckpointsRef() { return checkpoints; }
	
	bool isBetweenPoints(const QVector2D& carPos,
		const QVector2D& pointA,
		const QVector2D& pointB,
		float threshold = 5.0f) const;

	int isCarBetweenCheckpoints(const QVector2D& point) const; 

	const std::vector<TrackSegment>& getTrackSegments() const { return trackSegments; }
	std::vector<TrackSegment>& getTrackSegmentsRef() { return trackSegments; }
	bool hasSegments() const { return !trackSegments.empty(); }

private:
	std::vector<TrackPieces*> pieces;
	std::vector<QVector2D> centerLine;
	TrackEdges trackEdges;
	std::vector<BezierCurveData> bezierCurves; 
	std::vector<TrackSegment> trackSegments; 
	std::vector<CheckpointData> checkpoints; 
	float currentAngle;
	QVector2D currentPos;
	float trackWidth;
	float startAngle = 0.0;
	std::vector<int> piecesIntList;

	PitLane pitLane;
	int pitStartIndex = -1;  
	int pitEndIndex = -1;  

	std::vector<DecorPieces*> decors;

	std::vector<ChoixMapData> choixMapList;
	ChoixMapData currentChoixMapData;

};

#endif	
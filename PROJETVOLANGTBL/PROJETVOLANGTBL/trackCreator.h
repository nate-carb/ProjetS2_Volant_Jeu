#pragma once
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include "Track.h"

class TrackCreator : public QWidget
{
    Q_OBJECT

public:
    explicit TrackCreator(QWidget* parent = nullptr);
    void setTrack(const Track& track);
    void updateTrack(const Track& track);
    void addPiece(int pieceType);  // Add new piece to track

	void addDecor(int decor, int variant);// Add new decor piece to track
    int  findNearestCenterLineIndex(QVector2D pos);

    

    // track walls related functions
    void addBezierCurve(QVector2D start, QVector2D end);
    void addBezierCurveAtCenter();
    void toggleBezierEditMode(bool enabled);
    QVector2D evalBezier(const BezierCurveData& c, float t);
    bool getBezierEditMode() const { return m_bezierEditMode; }
    void drawBezierCurves(QPainter& painter);

	// track segment editing functions
    void drawTrackSegments(QPainter& painter);

    void clearTrack();              // Clear and start over
    Track getCurrentTrack() const { return currentTrack; }
    void loadTrack(const Track& track);
    std::vector<int> getPiecesList() const { return piecesList; }\


	QVector2D getCarPos() const { return carPos; }
    
    void closeTrack();// Close the track by connecting end to start
    
	// Segement editing functions for new track editor
    void addCurveSegment();
    void addStraightSegment();
    void removeLastSegment();

signals:
    void trackUpdated(const Track& track);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    Track currentTrack;
    std::vector<int> piecesList;  // Track the pieces added
	//std::vector<int> decorsList;  // Track the decors added
    void drawCar(QPainter& painter);
    void drawDecors(QPainter& painter);
    // Camera/view controls
    double zoom;
    QPointF offset;
    QPoint lastMousePos;
    bool dragging;

    // Helper functions
    QPointF worldToScreen(const QVector2D& worldPos);
    QVector2D screenToWorld(const QPointF& screenPos);
    void drawTrack(QPainter& painter);
    void drawSpriteTrack(QPainter& painter);
    void drawPiecePreview(QPainter& painter, int pieceType, const QVector2D& startPos, float startAngle);
    void calculateBounds(float& minX, float& maxX, float& minY, float& maxY);
    void rebuildTrack();

	
    
    
    //Car on track
    QVector2D carPos = QVector2D(20, 20);
    bool draggingCar = false;
    float carRadius = 4.0f;

    QVector2D dragOffset;
	//decor movement
    int  selectedDecorIndex = -1;
    bool isDraggingDecor = false;

	// Bezier curve editing
    int  m_selectedCurveIndex = -1;
    int  m_selectedPointIndex = -1; // 0=p0, 1=p1, 2=p2, 3=p3
    bool m_isDraggingBezier = false;
    bool m_bezierEditMode = false; // toggle on/off

    // Track segment state
    int  m_selectedSegIndex = -1;
    //int  m_selectedPointIndex = -1;
    bool m_isDraggingSegment = false;

};

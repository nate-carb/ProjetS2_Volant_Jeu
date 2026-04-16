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
    void addPiece(int pieceType);  

	void addDecor(int decor, int variant);
    int  findNearestCenterLineIndex(QVector2D pos);
	void rotateDecorRelative(float angle); 
	void rotateDecorExact(float angle); 
    
    void addBezierCurve(QVector2D start, QVector2D end);
    void addBezierCurveAtCenter();
    void toggleBezierEditMode(bool enabled);
    QVector2D evalBezier(const BezierCurveData& c, float t);
    bool getBezierEditMode() const { return m_bezierEditMode; }
    void drawBezierCurves(QPainter& painter);

    void drawTrackSegments(QPainter& painter);

    void clearTrack();             
    Track getCurrentTrack() const { return currentTrack; }
    void loadTrack(const Track& track);
    std::vector<int> getPiecesList() const { return piecesList; }\


	QVector2D getCarPos() const { return carPos; }
    
    void closeTrack();
    void autoDecors();
    void removeAutoDecors();

    void addCurveSegment();
    void addStraightSegment();
	void addPitSegment();
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
    std::vector<int> piecesList;  

    void drawCar(QPainter& painter);
    void drawDecors(QPainter& painter);
	void drawCheckpoints(QPainter& painter);

    double zoom;
    QPointF offset;
    QPoint lastMousePos;
    bool dragging;

    QPointF worldToScreen(const QVector2D& worldPos);
    QVector2D screenToWorld(const QPointF& screenPos);
    void drawTrack(QPainter& painter);
    void drawSpriteTrack(QPainter& painter);
    void drawPiecePreview(QPainter& painter, int pieceType, const QVector2D& startPos, float startAngle);
    void calculateBounds(float& minX, float& maxX, float& minY, float& maxY);
    void rebuildTrack();

    QVector2D carPos = QVector2D(20, 20);
    bool draggingCar = false;
    float carRadius = 4.0f;

    QVector2D dragOffset;
    int  selectedDecorIndex = -1;
    bool isDraggingDecor = false;

    int  m_selectedCurveIndex = -1;
    int  m_selectedPointIndex = -1; 
    bool m_isDraggingBezier = false;
    bool m_bezierEditMode = false;

    int  m_selectedSegIndex = -1;

    bool m_isDraggingSegment = false;

};

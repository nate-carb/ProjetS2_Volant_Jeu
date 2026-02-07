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
    void clearTrack();              // Clear and start over
    Track getCurrentTrack() const { return currentTrack; }
    void loadTrack(const Track& track);
    std::vector<int> getPiecesList() const { return piecesList; }

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

    // Camera/view controls
    double zoom;
    QPointF offset;
    QPoint lastMousePos;
    bool dragging;

    // Helper functions
    QPointF worldToScreen(const Vec2& worldPos);
    void drawTrack(QPainter& painter);
    void drawPiecePreview(QPainter& painter, int pieceType, const Vec2& startPos, float startAngle);
    void calculateBounds(float& minX, float& maxX, float& minY, float& maxY);
    void rebuildTrack();
};

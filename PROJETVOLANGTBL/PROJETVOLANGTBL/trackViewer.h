#pragma once
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include "map2d.h"
#include "Track.h"

class TrackViewer : public QWidget
{
    Q_OBJECT

public:
    explicit TrackViewer(QWidget* parent = nullptr);
    void setTrack(const Track& track);
    void updateTrack(const Track& track);
	

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    Track currentTrack;

    // Camera/view controls
    double zoom;
    QPointF offset;
    QPoint lastMousePos;
    bool dragging;

    // Helper functions
    QPointF worldToScreen(const QVector2D& worldPos);
    void drawTrack(QPainter& painter);
    void calculateBounds(float& minX, float& maxX, float& minY, float& maxY);

};

#pragma once
#include <QWidget>
#include <QPainter>
//#include <QMouseEvent>
//#include <QWheelEvent>
//#include "map2d.h"
#include "Track.h"

class TrackViewer : public QWidget
{
    Q_OBJECT

public:
    explicit TrackViewer(QWidget* parent = nullptr);
    void loadTrack(const Track& track);
    void setTrack(const Track& track);
    void updateTrack(const Track& track);
	Track getCurrentTrack() const { return currentTrack; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Track currentTrack;

    // Camera/view controls
    double zoom;
    QPointF offset;
   
    // Helper functions
    QPointF worldToScreen(const QVector2D& worldPos);
    void drawTrack(QPainter& painter);
    void calculateBounds(float& minX, float& maxX, float& minY, float& maxY);

};

#pragma once
#include <QWidget>
#include <QPainter>
#include "Track.h"

class TrackViewer : public QWidget
{
    Q_OBJECT

public:
    explicit TrackViewer(QWidget* parent = nullptr);
    void loadTrack(const Track& track);
    void setTrack(const Track& track);
    void drawSpriteTrack(QPainter& painter);
    void updateTrack(const Track& track);
	Track getCurrentTrack() const { return currentTrack; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Track currentTrack;

    // Camera
    double zoom;
    QPointF offset;
   
    // Helpers
    QPointF worldToScreen(const QVector2D& worldPos);
    void drawTrack(QPainter& painter);
    void calculateBounds(float& minX, float& maxX, float& minY, float& maxY);

};

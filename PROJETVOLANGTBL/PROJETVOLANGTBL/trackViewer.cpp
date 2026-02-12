#include "TrackViewer.h"
#include <QPainter>
#include <QPen>
#include <algorithm>

TrackViewer::TrackViewer(QWidget* parent)
    : QWidget(parent)
    , zoom(1.0)
    , offset(0, 0)
    , dragging(false)
{
    setMinimumSize(800, 600);
    setMouseTracking(true);
}

void TrackViewer::setTrack(const Track& track)
{
    currentTrack = track;

    // Auto-fit track to view
    if (!track.getCenterLine().empty()) {
        float minX, maxX, minY, maxY;
        calculateBounds(minX, maxX, minY, maxY);

        float rangeX = maxX - minX;
        float rangeY = maxY - minY;

        // Center the track
        offset = QPointF(-(minX + rangeX / 2), -(minY + rangeY / 2));

        // Set zoom to fit
        zoom = std::min(width() / rangeX, height() / rangeY) * 0.8;
    }

    update();
}

void TrackViewer::updateTrack(const Track& track)
{
    currentTrack = track;
    update();
}



void TrackViewer::calculateBounds(float& minX, float& maxX, float& minY, float& maxY)
{
    minX = minY = 1e9f;
    maxX = maxY = -1e9f;

    auto checkPoint = [&](const QVector2D& p) {
        minX = std::min(minX, p.x());
        maxX = std::max(maxX, p.x());
        minY = std::min(minY, p.y());
        maxY = std::max(maxY, p.y());
        };

    for (const auto& p : currentTrack.getCenterLine()) checkPoint(p);
    for (const auto& p : currentTrack.getTrackEdges().left) checkPoint(p);
    for (const auto& p : currentTrack.getTrackEdges().right) checkPoint(p);
}

QPointF TrackViewer::worldToScreen(const QVector2D& worldPos)
{
    double screenX = (worldPos.x() + offset.x()) * zoom + width() / 2.0;
    double screenY = (worldPos.y() + offset.y()) * zoom + height() / 2.0;
    return QPointF(screenX, screenY);
}



void TrackViewer::drawTrack(QPainter& painter)
{
    if (currentTrack.getCenterLine().empty())
        return;

    // Draw left edge (red)
    if (!currentTrack.getTrackEdges().left.empty()) {
        QPen leftPen(Qt::red, 3);
        painter.setPen(leftPen);

        for (size_t i = 1; i < currentTrack.getTrackEdges().left.size(); i++) {
            QPointF p1 = worldToScreen(currentTrack.getTrackEdges().left[i - 1]);
            QPointF p2 = worldToScreen(currentTrack.getTrackEdges().left[i]);
            painter.drawLine(p1, p2);
        }
    }

    // Draw right edge (blue)
    if (!currentTrack.getTrackEdges().right.empty()) {
        QPen rightPen(Qt::blue, 3);
        painter.setPen(rightPen);

        for (size_t i = 1; i < currentTrack.getTrackEdges().right.size(); i++) {
            QPointF p1 = worldToScreen(currentTrack.getTrackEdges().right[i - 1]);
            QPointF p2 = worldToScreen(currentTrack.getTrackEdges().right[i]);
            painter.drawLine(p1, p2);
        }
    }

    // Draw center line (yellow dashed)
    if (!currentTrack.getCenterLine().empty()) {
        QPen centerPen(Qt::yellow, 2, Qt::DashLine);
        painter.setPen(centerPen);

        for (size_t i = 1; i < currentTrack.getCenterLine().size(); i++) {
            QPointF p1 = worldToScreen(currentTrack.getCenterLine()[i - 1]);
            QPointF p2 = worldToScreen(currentTrack.getCenterLine()[i]);
            painter.drawLine(p1, p2);
        }
    }

    // Draw start position (green circle)
    QPointF startScreen = worldToScreen(QVector2D(0,0));
    painter.setPen(Qt::green);
    painter.setBrush(Qt::green);
    painter.drawEllipse(startScreen, 8, 8);

    // Draw text
    painter.setPen(Qt::white);
    painter.drawText(startScreen + QPointF(12, 0), "START");
}

void TrackViewer::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.fillRect(rect(), QColor(30, 30, 30));

    // Draw track
    drawTrack(painter);

    // Draw controls hint
    painter.setPen(Qt::white);
    painter.drawText(10, 20, "Mouse wheel: Zoom | Drag: Pan");
}

void TrackViewer::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && dragging == false) {
        dragging = true;
        lastMousePos = event->pos();
    }
    if (event->button() == Qt::RightButton && dragging == true) {
        dragging = false;
        lastMousePos = event->pos();
    }
}


void TrackViewer::mouseMoveEvent(QMouseEvent* event)
{
    if (dragging) {
        QPoint delta = event->pos() - lastMousePos;
        offset += QPointF(delta.x() / zoom, delta.y() / zoom);
        lastMousePos = event->pos();
        update();
    }
}

void TrackViewer::wheelEvent(QWheelEvent* event)
{
    double zoomFactor = 1.1;
    if (event->angleDelta().y() > 0) {
        zoom *= zoomFactor;
    }
    else {
        zoom /= zoomFactor;
    }
    update();
}

#include "TrackViewer.h"
#include <QPainter>
#include <QPen>
#include <algorithm>

TrackViewer::TrackViewer(QWidget* parent)
    : QWidget(parent)
    , zoom(1.0)
    , offset(0, 0)
{
    setMinimumSize(800, 600);
	currentTrack.loadFromFile("tracks/defaultTrack1.trk"); 
}



void TrackViewer::setTrack(const Track& track)
{
    currentTrack = track;

    if (!track.getCenterLine().empty()) {
        float minX, maxX, minY, maxY;
        calculateBounds(minX, maxX, minY, maxY);

        float rangeX = maxX - minX;
        float rangeY = maxY - minY;

        offset = QPointF(-(minX + rangeX / 2), -(minY + rangeY / 2));

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

    if (!currentTrack.getTrackEdges().left.empty()) {
        QPen leftPen(Qt::red, 3);
        painter.setPen(leftPen);

        for (size_t i = 1; i < currentTrack.getTrackEdges().left.size(); i++) {
            QPointF p1 = worldToScreen(currentTrack.getTrackEdges().left[i - 1]);
            QPointF p2 = worldToScreen(currentTrack.getTrackEdges().left[i]);
            painter.drawLine(p1, p2);
        }
    }

    if (!currentTrack.getTrackEdges().right.empty()) {
        QPen rightPen(Qt::blue, 3);
        painter.setPen(rightPen);

        for (size_t i = 1; i < currentTrack.getTrackEdges().right.size(); i++) {
            QPointF p1 = worldToScreen(currentTrack.getTrackEdges().right[i - 1]);
            QPointF p2 = worldToScreen(currentTrack.getTrackEdges().right[i]);
            painter.drawLine(p1, p2);
        }
    }

    if (!currentTrack.getCenterLine().empty()) {
        QPen centerPen(Qt::yellow, 2, Qt::DashLine);
        painter.setPen(centerPen);

        for (size_t i = 1; i < currentTrack.getCenterLine().size(); i++) {
            QPointF p1 = worldToScreen(currentTrack.getCenterLine()[i - 1]);
            QPointF p2 = worldToScreen(currentTrack.getCenterLine()[i]);
            painter.drawLine(p1, p2);
        }
    }

    QPointF startScreen = worldToScreen(QVector2D(0,0));
    painter.setPen(Qt::green);
    painter.setBrush(Qt::green);
    painter.drawEllipse(startScreen, 8, 8);

    painter.setPen(Qt::white);
    painter.drawText(startScreen + QPointF(12, 0), "START");
}

void TrackViewer::drawSpriteTrack(QPainter& painter)
{
    if (currentTrack.getCenterLine().empty())
        return;

    QTransform transform;
    transform.translate(width() / 2.0, height() / 2.0);
    transform.scale(zoom, zoom);
    transform.translate(offset.x(), offset.y());
    painter.setTransform(transform);

    auto centerLine = currentTrack.getCenterLine();
    for (size_t i = 1; i < centerLine.size(); i++) {
        QVector2D pos = centerLine[i];
        QVector2D prev = centerLine[i - 1];
        QVector2D dir = pos - prev;

        float angle = atan2(dir.y(), dir.x()) * 180.0f / M_PI;

        QPixmap sprite(currentTrack.getPieces()[i]->getSpritePath());

        if (sprite.isNull()) continue;

        painter.save();
        painter.translate(pos.x(), pos.y());
        painter.rotate(angle);
        painter.drawPixmap(
            -sprite.width() / 2,
            -sprite.height() / 2,
            sprite
        );
        painter.restore();
    }

        painter.resetTransform();

        painter.setPen(Qt::white);
        painter.drawText(10, height() - 10,
            QString("Pieces: %1").arg(currentTrack.getPiecesList().size()));
    
}
void TrackViewer::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(30, 30, 30));

    drawTrack(painter);

    
}

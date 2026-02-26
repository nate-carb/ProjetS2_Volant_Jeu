#include "trackCreator.h"
#include <QPainter> //Library for drawing on widgets
#include <QPen> //Library for defining pen styles (color, width, etc.)
#include <algorithm> // Library for algorithms like std::min and std::max
#include <QTransform>

TrackCreator::TrackCreator(QWidget* parent)
    : QWidget(parent)
    , zoom(1.0)
    , offset(0, 0)
    , dragging(false)
{
	setMinimumSize(800, 600); // Basic size for the track creator widget
	setMouseTracking(true); // Enable mouse tracking for interactive features

    // Initialize with empty track
    clearTrack();
}

void TrackCreator::loadTrack(const Track& track)
{
    // Get the pieces list from loaded track
	piecesList = track.getPiecesList(); // List of piece types that make up the track
    currentTrack = track;

    // Auto-fit view
    if (!track.getCenterLine().empty()) {
        float minX, maxX, minY, maxY;
        calculateBounds(minX, maxX, minY, maxY);

        float rangeX = maxX - minX;
        float rangeY = maxY - minY;

        offset = QPointF(-(minX + rangeX / 2), -(minY + rangeY / 2));

        if (rangeX > 0 && rangeY > 0) {
            zoom = std::min(width() / rangeX, height() / rangeY) * 0.8;
        }
    }

	emit trackUpdated(currentTrack); // Notify that a new track has been loaded
    update();
}
void TrackCreator::setTrack(const Track& track)
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
        if (rangeX > 0 && rangeY > 0) {
            zoom = std::min(width() / rangeX, height() / rangeY) * 0.8;
        }
    }

    update();
}

void TrackCreator::updateTrack(const Track& track)
{
    currentTrack = track;
    update();
}


void TrackCreator::addPiece(int pieceType)
{
    // Add piece to list
    piecesList.push_back(pieceType);

    // Rebuild track with new piece
    rebuildTrack();
}

void TrackCreator::clearTrack()
{
    piecesList.clear();
    currentTrack = Track();  // Empty track
    update();
}

void TrackCreator::rebuildTrack()
{
    if (piecesList.empty()) {
        currentTrack = Track();
    }
    else {
        currentTrack = Track(piecesList);
    }

    emit trackUpdated(currentTrack);
    update();
}
// Calculate the bounding box of the track for auto-fitting
void TrackCreator::calculateBounds(float& minX, float& maxX, float& minY, float& maxY) 
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
// Convert world coordinates to screen coordinates based on current zoom and offset
QPointF TrackCreator::worldToScreen(const QVector2D& worldPos)
{
    double screenX = (worldPos.x() + offset.x()) * zoom + width() / 2.0;
    double screenY = (worldPos.y() + offset.y()) * zoom + height() / 2.0;
    return QPointF(screenX, screenY);
}


void TrackCreator::drawTrack(QPainter& painter)
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
    drawCar(painter);
    // Draw start position (green circle)
    QPointF startScreen = worldToScreen(QVector2D(0, 0));
    painter.setPen(Qt::green);
    painter.setBrush(Qt::green);
    painter.drawEllipse(startScreen, 8, 8);

    // Draw text
    painter.setPen(Qt::white);
    painter.drawText(startScreen + QPointF(12, 0), "START");

    // Draw piece count
    painter.drawText(10, height() - 10,
        QString("Pieces: %1").arg(piecesList.size()));
}

void TrackCreator::drawCar(QPainter& painter) {
    if (currentTrack.isVector2DOnTrack(carPos))
        painter.setBrush(Qt::green);
    else
        painter.setBrush(Qt::red);

    // Convert car position from world to screen coordinates
    QPointF screenPos = worldToScreen(carPos);
    painter.drawEllipse(screenPos, 8, 8); //painter.drawEllipse(QPointF(carPos.x(), carPos.y()), 8, 8);
}

void TrackCreator::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.fillRect(rect(), QColor(30, 30, 30));

    // Draw track
    drawTrack(painter);

    // Draw car
    //drawCar(painter);

    // Draw controls hint
    painter.setPen(Qt::white);
    painter.drawText(10, 20, "Mouse wheel: Zoom | Left drag: Pan | Right click: Stop pan");

    
}

void TrackCreator::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && !dragging) {
        QVector2D mousePos(event->pos());
        QVector2D carScreenPos = QVector2D(worldToScreen(carPos));

        float distance = (mousePos - carScreenPos).length();

        if (distance <= carRadius)
        {
            draggingCar = true;
            // Store offset in world coordinates
            dragOffset = carPos - screenToWorld(event->pos());
        }
        else
        {
            dragging = true;
            lastMousePos = event->pos();
        }
    }
    if (event->button() == Qt::RightButton && dragging) {
        dragging = false;
    }
    
}

void TrackCreator::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        dragging = false;
    }
	draggingCar = false;
}

void TrackCreator::mouseMoveEvent(QMouseEvent* event)
{
    if (draggingCar)
    {
        QVector2D mouseWorldPos = screenToWorld(event->pos());
        carPos = mouseWorldPos + dragOffset;
        update();
    }
    else if (dragging) {
        QPoint delta = event->pos() - lastMousePos;
        offset += QPointF(delta.x() / zoom, delta.y() / zoom);
        lastMousePos = event->pos();
        update();
    }
}

void TrackCreator::wheelEvent(QWheelEvent* event)
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
QVector2D TrackCreator::screenToWorld(const QPointF& screenPos)
{
    double worldX = (screenPos.x() - width() / 2.0) / zoom - offset.x();
    double worldY = (screenPos.y() - height() / 2.0) / zoom - offset.y();
    return QVector2D(worldX, worldY);
}
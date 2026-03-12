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
    void createStartLine(); // Create a start line segment at the beginning of the track, aligned with the first segment's direction
	emit trackUpdated(currentTrack); // Notify that the track has been initialized
    update();
    
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



void TrackCreator::addDecor(int decor, int variant)
{
	currentTrack.addDecor(decor, variant);
	update();
    //// Rebuild track with new decor
    //rebuildTrack();
}

void TrackCreator::clearTrack()
{
    piecesList.clear();
    currentTrack = Track();  // Empty track
    update();
}

void TrackCreator::rebuildTrack()
{
    // Save decors before rebuilding - they belong to the layout, not the track geometry
    std::vector<DecorPieces*> savedDecors = currentTrack.getDecors();
    std::vector<BezierCurveData> savedBeziers = currentTrack.getBezierCurves();
    std::vector<TrackSegment> savedSegs = currentTrack.getTrackSegments();

    if (piecesList.empty()) {
        currentTrack = Track();
    }
    else {
        currentTrack = Track(piecesList);
    }
    

    // Restore after rebuild
    for (DecorPieces* d : savedDecors)
        currentTrack.addDecorDirect(d);
    for (BezierCurveData c : savedBeziers) 
        currentTrack.addBezierCurve(c);
    for (TrackSegment s : savedSegs)    
        currentTrack.addTrackSegment(s);

    currentTrack.buildFromSegments();
	//currentTrack.closeTrack(); // Ensure track is closed after rebuilding
    

    emit trackUpdated(currentTrack);
    update();
}

// In TrackCreator.cpp:
void TrackCreator::closeTrack()
{
    currentTrack.closeTrack();
    emit trackUpdated(currentTrack); // notify 3D viewer
    update();                        // repaint 2D view
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
    // Allow drawing if EITHER pieces OR segments exist
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
    // Draw pit lane if it exists
    if (currentTrack.hasPitLane()) {
        PitLane pit = currentTrack.getPitLane();

        // Pit lane center (orange dashed)
        QPen pitCenterPen(QColor(255, 165, 0), 2, Qt::DashLine);
        painter.setPen(pitCenterPen);
        for (size_t i = 1; i < pit.centerLine.size(); i++)
            painter.drawLine(worldToScreen(pit.centerLine[i - 1]),
                worldToScreen(pit.centerLine[i]));

        // Pit lane edges (dark orange)
        QPen pitEdgePen(QColor(200, 100, 0), 2);
        painter.setPen(pitEdgePen);
        for (size_t i = 1; i < pit.edges.left.size(); i++)
            painter.drawLine(worldToScreen(pit.edges.left[i - 1]),
                worldToScreen(pit.edges.left[i]));
        for (size_t i = 1; i < pit.edges.right.size(); i++)
            painter.drawLine(worldToScreen(pit.edges.right[i - 1]),
                worldToScreen(pit.edges.right[i]));

        // Entry curve (white)
        QPen curvePen(Qt::white, 2);
        painter.setPen(curvePen);
        for (size_t i = 1; i < pit.entryCurve.size(); i++)
            painter.drawLine(worldToScreen(pit.entryCurve[i - 1]),
                worldToScreen(pit.entryCurve[i]));

        // Pit lane edges (blue)
        QPen entryEdgePen(QColor(0, 0, 255), 2);
        painter.setPen(entryEdgePen);
        for (size_t i = 1; i < pit.entryCurveEdges.left.size(); i++)
            painter.drawLine(worldToScreen(pit.entryCurveEdges.left[i - 1]),
                worldToScreen(pit.entryCurveEdges.left[i]));
        for (size_t i = 1; i < pit.entryCurveEdges.right.size(); i++)
            painter.drawLine(worldToScreen(pit.entryCurveEdges.right[i - 1]),
                worldToScreen(pit.entryCurveEdges.right[i]));

        // Exit curve (white)
        for (size_t i = 1; i < pit.exitCurve.size(); i++)
            painter.drawLine(worldToScreen(pit.exitCurve[i - 1]),
                worldToScreen(pit.exitCurve[i]));

        // Pit lane edges (blue)
        QPen exitEdgePen(QColor(0, 0, 255), 2);
        painter.setPen(exitEdgePen);
        for (size_t i = 1; i < pit.exitCurveEdges.left.size(); i++)
            painter.drawLine(worldToScreen(pit.exitCurveEdges.left[i - 1]),
                worldToScreen(pit.exitCurveEdges.left[i]));
        for (size_t i = 1; i < pit.exitCurveEdges.right.size(); i++)
            painter.drawLine(worldToScreen(pit.exitCurveEdges.right[i - 1]),
                worldToScreen(pit.exitCurveEdges.right[i]));

        // Draw connector lines from main track to pit entry/exit
        //QPen connectorPen(Qt::white, 1, Qt::DotLine);
        //painter.setPen(connectorPen);
        //painter.drawLine(worldToScreen(currentTrack.getCenterLine()[currentTrack.getPitStartIndex()]),
        //    worldToScreen(pit.entryPoint));
        //painter.drawLine(worldToScreen(currentTrack.getCenterLine()[currentTrack.getPitEndIndex()]),
        //    worldToScreen(pit.exitPoint));
    }
    drawTrackSegments(painter);
    drawCheckpoints(painter);
	drawDecors(painter);
    drawBezierCurves(painter);
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

// Draw decors on the track (grandstands, trees, etc.)
void TrackCreator::drawDecors(QPainter& painter)
{
    for (int d = 0; d < (int)currentTrack.getDecors().size(); d++) {
        DecorPieces* decor = currentTrack.getDecors()[d];
        if (!decor) continue;

        QPointF dScreenPos = worldToScreen(decor->getInfo().pos);
        float   angle = decor->getInfo().angle;
        int modelType = decor->getInfo().modelType;

        // Pick color and label by model type
        QColor  decorColor;
        QString label;
        if (modelType == GRANDSTAND_INDEX) {
            decorColor = QColor(128, 0, 128); label = "GS";
			qDebug() << "(drawDecor) Grandstand decor at" << decor->getInfo().pos;
        }
        else if (modelType == GARAGE_INDEX) {
            decorColor = QColor(139, 69, 19); label = "GAR";
        }
        else if (modelType == TREES_INDEX) {
            decorColor = QColor(34, 139, 34);  label = "TRE";
        }
        else {
            decorColor = QColor(128, 128, 128); label = "?";
			qDebug() << "(drawDecor) Unknown decor type" << modelType << "at" << decor->getInfo().pos;
        }

        float w = decor->getInfo().width * zoom * decor->getInfo().scale;
        float h = decor->getInfo().length * zoom * decor->getInfo().scale;

        // Highlight selected decor with yellow border
        bool isSelected = (d == selectedDecorIndex);

        painter.save();
        painter.translate(dScreenPos);
        painter.rotate(qRadiansToDegrees(angle));

        painter.setBrush(QBrush(decorColor));
        painter.setPen(isSelected
            ? QPen(Qt::yellow, 3)
            : QPen(decorColor.darker(150), 1));
        painter.drawRect(-w / 2, -h / 2, w, h);

		painter.setPen(QPen(Qt::red, 1));
		painter.drawLine(0, 0, 0, -h / 2); // Draw small red line at decor center for angle reference

        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 8, QFont::Bold));
        painter.drawText(QRectF(-w / 2, -h / 2, w, h), Qt::AlignCenter, label);

        painter.restore();

        // Dotted line to nearest centerline point
        painter.setPen(QPen(decorColor, 1, Qt::DotLine));
        painter.drawLine(dScreenPos,
            worldToScreen(currentTrack.getCenterLine()[
                findNearestCenterLineIndex(decor->getInfo().pos)]));
    }
}

// Helper to find nearest centerline point to a position
int TrackCreator::findNearestCenterLineIndex(QVector2D pos)
{
    const auto& cl = currentTrack.getCenterLine();
    int nearest = 0;
    float minDist = std::numeric_limits<float>::max();
    for (int i = 0; i < (int)cl.size(); i++) {
        float d = (cl[i] - pos).length();
        if (d < minDist) { minDist = d; nearest = i; }
    }
    return nearest;
}

void TrackCreator::rotateDecorRelative(float angle)
{
   
	if (selectedDecorIndex < 0) {
        return;
    }
    DecorPieces* decor = currentTrack.getDecors()[selectedDecorIndex];
	//qDebug() << "(rotateDecorRelative) Current angle:" << decor->getInfo().angle << "Adding relative angle:" << angle;
	float newAngle = qRadiansToDegrees(decor->getInfo().angle) + angle;

	if (newAngle >= 360.0f) newAngle -= 360.0f;

    decor->setAngle(newAngle);
    emit trackUpdated(currentTrack);
	update();
}

void TrackCreator::rotateDecorExact(float angle)
{
    if (selectedDecorIndex < 0) {
        return;
    }
	DecorPieces* decor = currentTrack.getDecors()[selectedDecorIndex];
	decor->setAngle(angle);
    emit trackUpdated(currentTrack);
	update();
}

void TrackCreator::drawCheckpoints(QPainter& painter)
{
    const auto& checkpoints = currentTrack.getCheckpoints();
     QPen cpPen(Qt::cyan, 2);
     painter.setPen(cpPen);
	 // Draw lines between left and right edges at each checkpoint
     for (const auto& cp : checkpoints) {
         QVector2D p1 = QVector2D(
             currentTrack.getTrackEdges().left[cp.leftEdgeIndex].x(),
			 currentTrack.getTrackEdges().left[cp.leftEdgeIndex].y()
         );
		 QVector2D p2 = QVector2D(
             currentTrack.getTrackEdges().right[cp.rightEdgeIndex].x(),
			 currentTrack.getTrackEdges().right[cp.rightEdgeIndex].y()
         );
         painter.drawLine(
             worldToScreen(p1),
			 worldToScreen(p2) // left and right edge points of checkpoint
		 );
	 }
}
// ---------------------------
// Walls setup to use Bezier 
// ---------------------------
void TrackCreator::addBezierCurve(QVector2D start, QVector2D end)
{
    BezierCurveData c;
    c.p0 = start;
    c.p3 = end;
    c.p1 = start + (end - start) * 0.33f + QVector2D(0, -50);
    c.p2 = start + (end - start) * 0.66f + QVector2D(0, -50);
    currentTrack.addBezierCurve(c); // write directly to track
}
void TrackCreator::addBezierCurveAtCenter()
{
    QVector2D center = screenToWorld(QPoint(width() / 2, height() / 2));
    addBezierCurve(center - QVector2D(100, 0),
        center + QVector2D(100, 0));
    m_bezierEditMode = true;
    m_selectedCurveIndex = currentTrack.getBezierCurves().size() - 1;
    update();
}

void TrackCreator::toggleBezierEditMode(bool enabled)
{
    m_bezierEditMode = enabled;
    update();
}

QVector2D TrackCreator::evalBezier(const BezierCurveData& c, float t)
{
    float u = 1.0f - t;
    return c.p0 * (u * u * u)
        + c.p1 * (3 * u * u * t)
        + c.p2 * (3 * u * t * t)
        + c.p3 * (t * t * t);
}
// drawBezierCurves - read directly from track:
void TrackCreator::drawBezierCurves(QPainter& painter)
{
    const auto& curves = currentTrack.getBezierCurves(); // read from track
    for (int ci = 0; ci < (int)curves.size(); ci++) {
        const BezierCurveData& c = curves[ci];

        // ── Draw the curve ───────────────────────────────────
        QPen curvePen(QColor(0, 200, 255), 2);
        painter.setPen(curvePen);
        QPointF prev = worldToScreen(c.p0);
        for (int i = 1; i <= 40; i++) {
            float t = i / 40.0f;
            QPointF curr = worldToScreen(evalBezier(c, t));
            painter.drawLine(prev, curr);
            prev = curr;
        }
        bool isSelected = (ci == m_selectedCurveIndex);
        if (isSelected || m_bezierEditMode) {
            // ── Draw control lines ───────────────────────────
            QPen linePen(QColor(255, 255, 0, 120), 1, Qt::DashLine);
            painter.setPen(linePen);
            painter.drawLine(worldToScreen(c.p0), worldToScreen(c.p1));
            painter.drawLine(worldToScreen(c.p3), worldToScreen(c.p2));

            // ── Draw control points ──────────────────────────
            // p0 and p3 = endpoints (green)
            // p1 and p2 = handles (yellow)
            auto drawPoint = [&](QVector2D wp, QColor color, int idx) {
                QPointF sp = worldToScreen(wp);
                bool isHovered = (ci == m_selectedCurveIndex &&
                    idx == m_selectedPointIndex);
                painter.setBrush(isHovered ? Qt::white : color);
                painter.setPen(QPen(Qt::white, 1));
                painter.drawEllipse(sp, isHovered ? 10 : 7, isHovered ? 10 : 7);
                };

            drawPoint(c.p0, QColor(0, 255, 0), 0);
            drawPoint(c.p1, QColor(255, 255, 0), 1);
            drawPoint(c.p2, QColor(255, 255, 0), 2);
            drawPoint(c.p3, QColor(0, 255, 0), 3);
        }
    }
    

}
//------------------------------------------------
// Track segment editing functions for new track editor
// Each segment is either straight or curved, defined by its start/end points and optional control points for curves
// ------------------------------------------------
void TrackCreator::addCurveSegment()
{
    currentTrack.addCurveSegment();
    emit trackUpdated(currentTrack);
    update();
}

void TrackCreator::addStraightSegment()
{
    currentTrack.addStraightSegment();
    emit trackUpdated(currentTrack);
    update();
}

void TrackCreator::addPitSegment()
{
    currentTrack.addPitSegment();
    emit trackUpdated(currentTrack);
    update();
}

void TrackCreator::removeLastSegment()
{
    currentTrack.removeLastSegment();
    emit trackUpdated(currentTrack);
    update();
}
// drawTrackSegments - read directly from track:
void TrackCreator::drawTrackSegments(QPainter& painter)
{
    // ── Draw track surface from centerLine and edges ─────────
    const auto& cl = currentTrack.getCenterLine();
    const auto& left = currentTrack.getTrackEdges().left;
    const auto& right = currentTrack.getTrackEdges().right;

    if (cl.size() > 1) {
        // Left edge
        painter.setPen(QPen(Qt::red, 2));
        for (size_t i = 1; i < left.size(); i++)
            painter.drawLine(worldToScreen(left[i - 1]), worldToScreen(left[i]));

        // Right edge
        painter.setPen(QPen(Qt::blue, 2));
        for (size_t i = 1; i < right.size(); i++)
            painter.drawLine(worldToScreen(right[i - 1]), worldToScreen(right[i]));

        // Centerline
        painter.setPen(QPen(Qt::yellow, 1, Qt::DashLine));
        for (size_t i = 1; i < cl.size(); i++)
            painter.drawLine(worldToScreen(cl[i - 1]), worldToScreen(cl[i]));
    }
    // ── Draw control handles ─────────────────────────────────
    const auto& segs = currentTrack.getTrackSegments();

    for (int si = 0; si < (int)segs.size(); si++) {
        const TrackSegment& s = segs[si];
        bool        isSelected = (si == m_selectedSegIndex);

        // ── Reuse same drawPoint lambda as wall bezier ───────
        auto drawPoint = [&](QVector2D wp, QColor color, int idx) {
            QPointF sp = worldToScreen(wp);
            bool isHovered = (si == m_selectedSegIndex &&
                idx == m_selectedPointIndex);
            painter.setBrush(isHovered ? Qt::white : color);
            painter.setPen(QPen(Qt::white, 1));
            painter.drawEllipse(sp, isHovered ? 10 : 7,
                isHovered ? 10 : 7);
            };

        // End point always visible
        drawPoint(s.end, QColor(0, 255, 0), 0);

        // Curve handles only when selected
        if (isSelected && s.type == TrackSegmentType::CURVE_TRACK) {
            painter.setPen(QPen(QColor(255, 255, 0, 150), 1, Qt::DashLine));
            painter.drawLine(worldToScreen(s.start), worldToScreen(s.cp1));
            painter.drawLine(worldToScreen(s.end), worldToScreen(s.cp2));
            drawPoint(s.cp1, QColor(255, 255, 0), 1);
            drawPoint(s.cp2, QColor(255, 255, 0), 2);
        }

        // Label S or C at midpoint
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 8));
        QPointF mid = worldToScreen((s.start + s.end) / 2.0f);
        painter.drawText(mid + QPointF(5, -5),
            s.type == TrackSegmentType::STRAIGHT_TRACK ? "S" : "C");
    }
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
    if (event->button() == Qt::LeftButton) {
        QVector2D mousePos(event->pos());

        // ── Check decors first ───────────────────────────────
        for (int d = 0; d < (int)currentTrack.getDecors().size(); d++) {
            DecorPieces* decor = currentTrack.getDecors()[d];
            QPointF screenPos = worldToScreen(decor->getInfo().pos);
            float dist = QVector2D(
                event->pos().x() - screenPos.x(),
                event->pos().y() - screenPos.y()).length();
            float hitRadius = qMax(
                decor->getInfo().width * zoom * decor->getInfo().scale,
                decor->getInfo().length * zoom * decor->getInfo().scale);

            if (dist < hitRadius) {
                selectedDecorIndex = d;
                isDraggingDecor = true;
                update();
                return; // decor hit - don't pan or drag car
            }
        }
        // ── Check bezier control points first ───────────────────
        if (m_bezierEditMode && event->button() == Qt::LeftButton) {
            float hitRadius = 12.0f;
            const auto& curves = currentTrack.getBezierCurves(); // READ FROM TRACK

            for (int ci = 0; ci < (int)curves.size(); ci++) {
                const BezierCurveData& c = curves[ci];
                QVector2D pts[4] = { c.p0, c.p1, c.p2, c.p3 };

                for (int pi = 0; pi < 4; pi++) {
                    QPointF sp = worldToScreen(pts[pi]);
                    float dist = QVector2D(
                        event->pos().x() - sp.x(),
                        event->pos().y() - sp.y()).length();

                    if (dist < hitRadius) {
                        m_selectedCurveIndex = ci;
                        m_selectedPointIndex = pi;
                        m_isDraggingBezier = true;
                        update();
                        return;
                    }
                }
            }
            m_selectedCurveIndex = -1;
            m_selectedPointIndex = -1;
        }
        // ── Check track segments ─────────────────────────────────
        if (event->button() == Qt::LeftButton) {
            const auto& segs = currentTrack.getTrackSegments();
            float hitRadius = 12.0f;

            for (int si = 0; si < (int)segs.size(); si++) {
                const TrackSegment& s = segs[si];

                auto checkPoint = [&](QVector2D wp, int idx) -> bool {
                    QPointF sp = worldToScreen(wp);
                    float   dist = QVector2D(
                        event->pos().x() - sp.x(),
                        event->pos().y() - sp.y()).length();
                    if (dist < hitRadius) {
                        m_selectedSegIndex = si;
                        m_selectedPointIndex = idx;
                        m_isDraggingSegment = true;
                        update();
                        return true;
                    }
                    return false;
                    };

                if (checkPoint(s.end, 0)) return;

                // Curve handles only when selected
                if (si == m_selectedSegIndex &&
                    s.type == TrackSegmentType::CURVE_TRACK) {
                    if (checkPoint(s.cp1, 1)) return;
                    if (checkPoint(s.cp2, 2)) return;
                }
            }
        }
        // ── Check car ────────────────────────────────────────
        QVector2D carScreenPos = QVector2D(worldToScreen(carPos));
        float carDist = (mousePos - carScreenPos).length();
        if (carDist <= carRadius) {
            draggingCar = true;
            dragOffset = carPos - screenToWorld(event->pos());
            return; // car hit - don't pan
        }

        // ── Nothing hit - pan the view ───────────────────────
        if (!dragging) {
            dragging = true;
            lastMousePos = event->pos();
        }

        
    }

    if (event->button() == Qt::RightButton) {
        dragging = false;
        selectedDecorIndex = -1;
        update();
    }
    
}

void TrackCreator::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        dragging = false;
    }
	draggingCar = false;
    isDraggingDecor = false;
    isDraggingDecor = false;
    m_isDraggingBezier = false;  // no track data here, stays the same
    m_isDraggingSegment = false;  
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
    if (isDraggingDecor && selectedDecorIndex >= 0) {
        currentTrack.getDecors()[selectedDecorIndex]->setPos(
            screenToWorld(event->pos()));
        update();
    }
    // mouseMoveEvent - write directly to track:
    if (m_isDraggingBezier && m_selectedCurveIndex >= 0
        && m_selectedPointIndex >= 0) {
        QVector2D worldPos = screenToWorld(event->pos());
        // Get reference to curve in track directly
        BezierCurveData& c = currentTrack.getBezierCurveRef(m_selectedCurveIndex);
        switch (m_selectedPointIndex) {
        case 0: c.p0 = worldPos; break;
        case 1: c.p1 = worldPos; break;
        case 2: c.p2 = worldPos; break;
        case 3: c.p3 = worldPos; break;
        }
        update();
        return;
    }
    // Track segment drag - same pattern as bezier wall drag
    if (m_isDraggingSegment && m_selectedSegIndex >= 0) {
        QVector2D worldPos = screenToWorld(event->pos());
        TrackSegment& s =
            currentTrack.getTrackSegmentsRef()[m_selectedSegIndex];

        switch (m_selectedPointIndex) {
        case 0: // end point
            s.end = worldPos;
            if (s.type == TrackSegmentType::STRAIGHT_TRACK) {
                s.cp1 = s.start;
                s.cp2 = s.end;
            }
            // Keep next segment linked
            if (m_selectedSegIndex + 1 <
                (int)currentTrack.getTrackSegmentsRef().size()) {
                currentTrack.getTrackSegmentsRef()
                    [m_selectedSegIndex + 1].start = worldPos;
            }
            break;
        case 1: s.cp1 = worldPos; break;
        case 2: s.cp2 = worldPos; break;
        }

        currentTrack.buildFromSegments();
        emit trackUpdated(currentTrack);
        update();
        return;
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
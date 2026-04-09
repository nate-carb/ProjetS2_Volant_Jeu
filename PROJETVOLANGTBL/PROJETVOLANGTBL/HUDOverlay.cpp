#include "HUDOverlay.h"
#include <QPainter>
#include <QPainterPath>
#include <QTime>
#include <QFontDatabase>

HUDOverlay::HUDOverlay(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    // Charge les images depuis les ressources Qt
    m_pixLap = QPixmap("./imagehud/lap.png");
    m_pixLapTime = QPixmap("./imagehud/laptime.png");
    m_pixSpeedRpm = QPixmap("./imagehud/vitesserpmvide.png");
    m_pixWarning = QPixmap("./imagehud/warning.png");
    QFontDatabase::addApplicationFont("./font/PressStart2P-Regular.ttf");
}

void HUDOverlay::updateData(float carburant, float nos, float tireWear,
    Vehicule::Weather weather,
    float speedKmh, float rpm, float maxRpm,
    int currentLap, int totalLaps,
    qint64 bestLapMs, qint64 currentLapMs, qint64 deltaMs,
    bool warning, const RaceTimes* raceTimes,
    const Track* track, const PitStop* pitStop,
    QVector2D carPos, float carAngle)
{
    m_carburant = carburant;
    m_nos = nos;
    m_tireWear = tireWear;
    m_weather = weather;
    m_speedKmh = speedKmh;
    m_rpm = rpm;
    m_maxRpm = maxRpm;
    m_currentLap = currentLap;
    m_totalLaps = totalLaps;
    m_bestLapMs = bestLapMs;
    m_currentLapMs = currentLapMs;
    m_deltaMs = deltaMs;
    m_warning = warning;
    m_raceTimes = raceTimes;
    m_track = track;
    m_pitStop = pitStop;
    m_carPos = carPos;
    m_carAngle = carAngle;
    update();
}

void HUDOverlay::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (m_paused) return;  // transparent quand pause

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::TextAntialiasing, false);

    // ── Pluie ─────────────────────────────────────────────────────────────
    if (!m_paused && (m_weather == Vehicule::RAINY || m_weather == Vehicule::STORMY)) {
        int   numDrops = (m_weather == Vehicule::STORMY) ? 150 : 75;
        float penWidth = (m_weather == Vehicule::STORMY) ? 2.5f : 1.5f;
        painter.setPen(QPen(QColor(150, 150, 255, 150), penWidth));
        srand(QTime::currentTime().msec());
        for (int i = 0; i < numDrops; i++) {
            int x = rand() % width(), y = rand() % height();
            int len = (m_weather == Vehicule::STORMY) ? 20 : 12;
            painter.drawLine(x, y, x - 3, y + len);
        }
    }

    // ── Widgets ───────────────────────────────────────────────────────────
    drawLapCounter(painter);
    drawLapTimes(painter);
    drawSpeedRpm(painter);
    drawBoostBar(painter);
    drawMinimap(painter);

    if (m_warning) drawWarning(painter);
}

// ─────────────────────────────────────────────────────────────────────────────
// LAP COUNTER — image lap.png + texte "2/3" par-dessus
// ─────────────────────────────────────────────────────────────────────────────
void HUDOverlay::drawLapCounter(QPainter& p)
{
    const int W = 120, H = 90;
    int x = width() - W - 15, y = 15;

    // Image de fond
    p.drawPixmap(x, y, W, H, m_pixLap);

    // Numéro de lap centré sur l'image
    // (ajuste les offsets selon la position exacte du chiffre dans lap.png)
    p.setRenderHint(QPainter::TextAntialiasing, false);
    p.setFont(QFont("Press Start 2P", 16));
    p.setPen(Qt::white);
    QString lapStr = QString("%1  %2").arg(m_currentLap).arg(m_totalLaps);
    p.drawText(QRect(x, y + 35, W, 40), Qt::AlignCenter, lapStr);
}

// ─────────────────────────────────────────────────────────────────────────────
// LAP TIMES — image laptime.png + textes BEST/LAP/DELTA par-dessus
// ─────────────────────────────────────────────────────────────────────────────
void HUDOverlay::drawLapTimes(QPainter& p)
{
    if (!m_raceTimes) return;

    const int W = 200, H = 110;
    int x = width() - W - 15;
    int y = 15 + 90 + 6; // juste sous le lap counter

    // Image de fond
    p.drawPixmap(x, y, W, H, m_pixLapTime);

    // Prépare les valeurs
    QString bestStr = (m_bestLapMs >= 0)
        ? m_raceTimes->formatMs(m_bestLapMs) : "--:--.---";
    QString currentStr = m_raceTimes->formatMs(m_currentLapMs);

    QString deltaStr;
    QColor  deltaColor;
    if (m_bestLapMs < 0) {
        deltaStr = "--:--.---";
        deltaColor = Qt::white;
    }
    else {
        bool ahead = m_deltaMs <= 0;
        deltaStr = (ahead ? "-" : "+") + m_raceTimes->formatMs(std::abs(m_deltaMs));
        deltaColor = ahead ? QColor(80, 220, 80) : QColor(220, 80, 80);
    }

    // Chaque ligne fait ~H/3 de haut dans l'image
    // Les offsets sont calibrés pour laptime.png (ajuste si nécessaire)
    const int ROW_H = H / 3;
    struct Row { QString value; QColor color; };
    Row rows[3] = {
        { bestStr,    QColor(80, 220, 80) },  // BEST  — vert
        { currentStr, Qt::white           },  // LAP   — blanc
        { deltaStr,   deltaColor          },  // DELTA — vert/rouge
    };

    p.setRenderHint(QPainter::TextAntialiasing, false);
    p.setFont(QFont("Press Start 2P", 8));
    int offsets[3] = { 5, 38, 68 }; // BEST, LAP, DELTA — tweake ces valeurs
    for (int i = 0; i < 3; i++) {
        QRect rowRect(x + 2, y + offsets[i], W - 14, ROW_H);
        p.setPen(rows[i].color);
        p.drawText(rowRect, Qt::AlignRight | Qt::AlignVCenter, rows[i].value);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// VITESSE + RPM — image vitesserpmvide.png + chiffre + segments par-dessus
// ─────────────────────────────────────────────────────────────────────────────
void HUDOverlay::drawSpeedRpm(QPainter& p)
{
    const int W = 220, H = 90;
    int x = width() - W - 15;
    int y = height() - H - 15;

    // Image de fond (cadre vide avec labels KM/H et RPM)
    p.drawPixmap(x, y, W, H, m_pixSpeedRpm);

    // ── Vitesse (gros chiffre, haut droite dans le cadre) ──
    p.setRenderHint(QPainter::TextAntialiasing, false);
    p.setFont(QFont("Press Start 2P", 18));
    p.setPen(Qt::yellow);
    // Zone du chiffre : occupe la moitié droite du haut du cadre
    p.drawText(QRect(x + 2, y + 14, W - 70, 45),
        Qt::AlignRight | Qt::AlignVCenter,
        QString::number((int)m_speedKmh));

    // ── Barre RPM (bas du cadre, à droite de "RPM") ──
    const int NUM_SEG = 14;
    const int SEG_W = 9, SEG_H = 12, SEG_GAP = 1;
    // Ajuste barX/barY pour coller exactement sur les cases vides de l'image
    int barX = x + 64;
    int barY = y + H - SEG_H - 14;

    float rpmRatio = (m_rpm - 800.0f) / std::max(m_maxRpm - 800.0f, 1.0f);
    int filledCount = (int)(rpmRatio * NUM_SEG);
    filledCount = std::clamp(filledCount, 0, NUM_SEG);

    for (int i = 0; i < NUM_SEG; i++) {
        QRect seg(barX + i * (SEG_W + SEG_GAP), barY, SEG_W, SEG_H);

        if (i < filledCount) {
            // Vert → Jaune → Rouge
            float t = (float)i / (NUM_SEG - 1);
            QColor c;
            if (t < 0.5f)
                c = QColor(int(80 + 175 * t * 2), 220, 0);
            else
                c = QColor(220, int(220 - 200 * (t - 0.5f) * 2), 0);
            p.setBrush(c);
            p.setPen(c.lighter(130));
        }
        else {
            // Transparent pour laisser voir les cases vides de l'image
            p.setBrush(Qt::NoBrush);
            p.setPen(Qt::NoPen);
        }
        p.drawRoundedRect(seg, 2, 2);
    }
}
// ─────────────────────────────────────────────────────────────────────────────
// BOOST BAR — image boostvide.png + segments orange/jaune (bas gauche)
// Même principe que la barre RPM
// ─────────────────────────────────────────────────────────────────────────────
void HUDOverlay::drawBoostBar(QPainter& p)
{
    if (m_pixBoostEmpty.isNull()) return;

    const int H = 140;
    int W = m_pixBoostEmpty.width() * H / m_pixBoostEmpty.height();

    // ── Aligné à droite, au-dessus du panneau KM/H ──
    const int speedPanelW = 220, speedPanelH = 90;
    int x = width() - speedPanelW - 15;   // même bord gauche que KM/H
    int y = height() - speedPanelH - 15 - H - 8; // juste au-dessus

    p.save();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 80)); // augmente le 120 pour plus opaque
    p.drawRoundedRect(x +12, y +45, W , H-100, 8, 8);
    p.restore();

    p.drawPixmap(x, y, W, H, m_pixBoostEmpty);

    const int NUM_SEG = 8;
    const int SEG_W = 9, SEG_H = 12, SEG_GAP = 1;
    int barX = x + (int)(W * 0.6f); // ← diminue si encore trop à droite
    int barY = y + (H - SEG_H) / 2;

    float nosRatio = m_nos / 100.0f;
    int filledCount = std::clamp((int)(nosRatio * NUM_SEG), 0, NUM_SEG);

    if (filledCount > 0) {
        p.save();
        QRadialGradient glow(barX + NUM_SEG * (SEG_W + SEG_GAP) / 2,
            barY + SEG_H / 2, 55);
        glow.setColorAt(0.0, QColor(255, 140, 0, 70));
        glow.setColorAt(1.0, Qt::transparent);
        p.setBrush(glow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(barX - 10, barY - 12,
            NUM_SEG * (SEG_W + SEG_GAP) + 20, SEG_H + 24);
        p.restore();
    }

    for (int i = 0; i < NUM_SEG; i++) {
        QRect seg(barX + i * (SEG_W + SEG_GAP), barY, SEG_W, SEG_H);
        if (i < filledCount) {
            float t = (float)i / (NUM_SEG - 1);
            QColor c(220, int(80 + 140 * t), 0);
            p.setBrush(c);
            p.setPen(c.lighter(130));
        }
        else {
            p.setBrush(Qt::NoBrush);
            p.setPen(Qt::NoPen);
        }
        p.drawRoundedRect(seg, 2, 2);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// WARNING — image warning.png, clignotante, centrée
// ─────────────────────────────────────────────────────────────────────────────
void HUDOverlay::drawWarning(QPainter& p)
{
    // Clignote toutes les 400ms
    if ((QTime::currentTime().msec() / 400) % 2 == 0) return;

    const int SIZE = 80;
    int x = width() / 2 - SIZE / 2;
    int y = height() / 2 - SIZE - 20;

    p.drawPixmap(x, y, SIZE, SIZE, m_pixWarning);
}

// ── Barres existantes (inchangées) ───────────────────────────────────────────
void HUDOverlay::drawBar(QPainter& painter, int x, int y, int w, int h,
    float value, QColor color, const QString& label)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(60, 60, 60, 200));
    painter.drawRoundedRect(x, y, w, h, 4, 4);
    int fillW = (int)(w * value / 100.0f);
    if (fillW > 0) {
        painter.setBrush(color);
        painter.drawRoundedRect(x, y, fillW, h, 4, 4);
    }
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(x + 5, y + h - 5, label);
    painter.setFont(QFont("Arial", 9));
    painter.drawText(x + w - 35, y + h - 5, QString("%1%").arg((int)value));
}

//void HUDOverlay::drawBoostBar(QPainter& painter, int x, int y, float nosRatio)
//{
//    const int NUM_SEGMENTS = 8, SEG_W = 16, SEG_H = 18, SEG_GAP = 3, CORNER = 2;
//    int filledCount = (int)(nosRatio * NUM_SEGMENTS);
//
//    painter.setPen(QColor(255, 200, 0));
//    painter.setFont(QFont("Arial", 10, QFont::Bold));
//    painter.drawText(x, y + SEG_H - 2, "⚡ BOOST");
//
//    int barX = x + 90;
//
//    if (nosRatio > 0.0f) {
//        painter.save();
//        QRadialGradient glow(barX + NUM_SEGMENTS * (SEG_W + SEG_GAP) / 2,
//            y + SEG_H / 2, 50);
//        glow.setColorAt(0.0, QColor(255, 150, 0, 60));
//        glow.setColorAt(1.0, Qt::transparent);
//        painter.setBrush(glow);
//        painter.setPen(Qt::NoPen);
//        painter.drawEllipse(barX - 10, y - 10,
//            NUM_SEGMENTS * (SEG_W + SEG_GAP) + 20, SEG_H + 20);
//        painter.restore();
//    }
//
//    for (int i = 0; i < NUM_SEGMENTS; i++) {
//        QRect seg(barX + i * (SEG_W + SEG_GAP), y, SEG_W, SEG_H);
//        if (i < filledCount) {
//            QLinearGradient grad(seg.topLeft(), seg.bottomRight());
//            grad.setColorAt(0.0, QColor(255, 180, 0));
//            grad.setColorAt(1.0, QColor(210, 70, 0));
//            painter.setBrush(grad);
//            painter.setPen(QColor(255, 210, 50, 200));
//        }
//        else {
//            painter.setBrush(QColor(30, 20, 0, 150));
//            painter.setPen(QColor(80, 55, 0, 120));
//        }
//        painter.drawRoundedRect(seg, CORNER, CORNER);
//    }
//}

void HUDOverlay::drawMinimap(QPainter& p)
{
    if (!m_track || m_track->getCenterLine().empty()) return;

    // ── Calculer les bounds de la piste ──────────────────────
    float minX = 1e9f, maxX = -1e9f;
    float minY = 1e9f, maxY = -1e9f;

    //auto checkPoint = [&](const QVector2D& p) {
    //    minX = std::min(minX, p.x()); maxX = std::max(maxX, p.x());
    //    minY = std::min(minY, p.y()); maxY = std::max(maxY, p.y());
    //    };
    auto checkPoint = [&](const QVector2D& p) {
        minX = qMin(minX, p.x()); maxX = qMax(maxX, p.x());
        minY = qMin(minY, p.y()); maxY = qMax(maxY, p.y());
        };
    for (const auto& p : m_track->getCenterLine())       checkPoint(p);
    for (const auto& p : m_track->getTrackEdges().left)  checkPoint(p);
    for (const auto& p : m_track->getTrackEdges().right) checkPoint(p);

    float padding = (maxX - minX) * 0.05f;
    minX -= padding; maxX += padding;
    minY -= padding; maxY += padding;

    float rangeX = maxX - minX;
    float rangeY = maxY - minY;
    if (rangeX <= 0 || rangeY <= 0) return;

    // ── Calcul proportionnel ──────────────────────────────────
    const float MAX_SIZE = 250.0f;
    const int MARGIN = 15;

    float scale = qMin(MAX_SIZE / rangeX, MAX_SIZE / rangeY);
    float minimapWidth = rangeX * scale;
    float minimapHeight = rangeY * scale;

    QRectF minimapRect(
        MARGIN,
        height() - minimapHeight - MARGIN,
        minimapWidth,
        minimapHeight
    );

    // ── Projection monde → mini-map ──────────────────────────
    auto toMinimap = [&](const QVector2D& p) -> QPointF {
        float nx = (p.x() - minX) / rangeX;
        float ny = (p.y() - minY) / rangeY;
        return QPointF(
            minimapRect.left() + nx * minimapRect.width(),
            minimapRect.top() + ny * minimapRect.height()
        );
        };

    // ── Fond semi-transparent ─────────────────────────────────
    p.setBrush(QColor(0, 0, 0, 160));
    p.setPen(QPen(QColor(255, 255, 255, 80), 1));
    p.drawRoundedRect(minimapRect, 8, 8);

    // ── Clipping ─────────────────────────────────────────────
    p.save();
    QPainterPath clipPath;
    clipPath.addRoundedRect(minimapRect, 8, 8);
    p.setClipPath(clipPath);

    // ── Surface de la piste ───────────────────────────────────
    const auto& left = m_track->getTrackEdges().left;
    const auto& right = m_track->getTrackEdges().right;

    if (left.size() >= 2 && right.size() >= 2) {
        QPolygonF trackPoly;
        for (const auto& p : left)
            trackPoly << toMinimap(p);
        for (int i = (int)right.size() - 1; i >= 0; i--)
            trackPoly << toMinimap(right[i]);

        p.setBrush(QColor(80, 80, 80, 220));
        p.setPen(Qt::NoPen);
        p.drawPolygon(trackPoly);
    }

    // ── Pit lane ──────────────────────────────────────────────
    if (m_track->hasPitLane()) {
        PitLane pit = m_track->getPitLane();

        auto drawEdgePoly = [&](const std::vector<QVector2D>& edgeL,
            const std::vector<QVector2D>& edgeR) {
                if (edgeL.empty() || edgeR.empty()) return;
                QPolygonF poly;
                for (const auto& p : edgeL)  poly << toMinimap(p);
                for (int i = (int)edgeR.size() - 1; i >= 0; i--)
                    poly << toMinimap(edgeR[i]);
                p.setBrush(QColor(255, 165, 0, 200));  // orange
                p.setPen(Qt::NoPen);
                p.drawPolygon(poly);
            };

        drawEdgePoly(pit.edges.left, pit.edges.right);
        drawEdgePoly(pit.entryCurveEdges.left, pit.entryCurveEdges.right);
        drawEdgePoly(pit.exitCurveEdges.left, pit.exitCurveEdges.right);
    }

    // ── Ligne centrale ────────────────────────────────────────
    const auto& center = m_track->getCenterLine();
    p.setPen(QPen(QColor(255, 255, 255, 80), 1, Qt::DashLine));
    for (size_t i = 1; i < center.size(); i++)
        p.drawLine(toMinimap(center[i - 1]), toMinimap(center[i]));

    // ── Joueur ────────────────────────────────────────────────
    p.setBrush(Qt::red);
    p.setPen(Qt::NoPen);
    p.drawEllipse(toMinimap(m_carPos), 3, 3);

    // ── Pit stop box ──────────────────────────────────────────
    QRect pitRect = m_pitStop->getRect();
    const float PIXELS_PER_METER = 5.0f;
    QVector2D pitCenter(
        pitRect.center().x() / PIXELS_PER_METER,
        pitRect.center().y() / PIXELS_PER_METER
    );
    p.setBrush(Qt::yellow);
    p.setPen(Qt::NoPen);
    QPointF pitMiniPos = toMinimap(pitCenter);
    p.drawRect(QRectF(pitMiniPos.x() - 4, pitMiniPos.y() - 4, 8, 8));

    p.restore();
}
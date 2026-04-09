#include "HUDOverlay.h"
#include <QPainter>
#include <QPainterPath>
#include <QTime>

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
}

void HUDOverlay::updateData(float carburant, float nos, float tireWear,
    Vehicule::Weather weather,
    float speedKmh, float rpm, float maxRpm,
    int currentLap, int totalLaps,
    qint64 bestLapMs, qint64 currentLapMs, qint64 deltaMs,
    bool warning, const RaceTimes* raceTimes)
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
    update();
}

void HUDOverlay::paintEvent(QPaintEvent*)
{
    if (m_paused) {
        QPainter painter(this);
        painter.fillRect(rect(), QColor(0, 0, 0, 0));  // transparent quand pause
        return;
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // ── HUD gauche (barres) ───────────────────────────────────────────────
    QPainterPath bg;
    bg.addRoundedRect(15, 15, 200, 160, 12, 12);
    painter.fillPath(bg, QColor(0, 0, 0, 150));

    QColor fuelColor = (m_carburant > 50) ? QColor(80, 220, 80)
        : (m_carburant > 25) ? QColor(255, 165, 0)
        : QColor(220, 50, 50);
    drawBar(painter, 25, 30, 170, 22, m_carburant, fuelColor, "Carburant");
    drawBoostBar(painter, 25, 70, m_nos / 100.0f);
    QColor tireColor = (m_tireWear > 60) ? QColor(80, 220, 80)
        : (m_tireWear > 30) ? QColor(255, 165, 0)
        : QColor(220, 50, 50);
    drawBar(painter, 25, 110, 170, 22, m_tireWear, tireColor, "Pneus");

    // ── Météo ─────────────────────────────────────────────────────────────
    QString weatherText;
    QColor  weatherColor;
    switch (m_weather) {
    case Vehicule::SUNNY:  weatherText = "☀ Ensoleillé"; weatherColor = QColor(255, 220, 0);   break;
    case Vehicule::RAINY:  weatherText = "🌧 Pluie";     weatherColor = QColor(100, 180, 255); break;
    case Vehicule::STORMY: weatherText = "⛈ Tempête";   weatherColor = QColor(180, 100, 255); break;
    }
    painter.setPen(weatherColor);
    painter.setFont(QFont("Arial", 11, QFont::Bold));
    painter.drawText(25, 155, weatherText);

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

    // ── Widgets images ────────────────────────────────────────────────────
    drawLapCounter(painter);
    drawLapTimes(painter);
    drawSpeedRpm(painter);
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
    p.setFont(QFont("Courier", 20, QFont::Bold));
    p.setPen(Qt::white);
    QString lapStr = QString("%1/%2").arg(m_currentLap).arg(m_totalLaps);
    p.drawText(QRect(x, y + 40, W, 40), Qt::AlignCenter, lapStr);
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

    p.setFont(QFont("Courier", 11, QFont::Bold));
    for (int i = 0; i < 3; i++) {
        // Aligne à droite dans chaque ligne de l'image
        QRect rowRect(x, y + i * ROW_H, W - 10, ROW_H);
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
    p.setFont(QFont("Courier", 28, QFont::Bold));
    p.setPen(Qt::white);
    // Zone du chiffre : occupe la moitié droite du haut du cadre
    p.drawText(QRect(x + 60, y + 5, W - 70, 45),
        Qt::AlignRight | Qt::AlignVCenter,
        QString::number((int)m_speedKmh));

    // ── Barre RPM (bas du cadre, à droite de "RPM") ──
    const int NUM_SEG = 16;
    const int SEG_W = 9, SEG_H = 14, SEG_GAP = 2;
    // Ajuste barX/barY pour coller exactement sur les cases vides de l'image
    int barX = x + 52;
    int barY = y + H - SEG_H - 8;

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

void HUDOverlay::drawBoostBar(QPainter& painter, int x, int y, float nosRatio)
{
    const int NUM_SEGMENTS = 8, SEG_W = 16, SEG_H = 18, SEG_GAP = 3, CORNER = 2;
    int filledCount = (int)(nosRatio * NUM_SEGMENTS);

    painter.setPen(QColor(255, 200, 0));
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(x, y + SEG_H - 2, "⚡ BOOST");

    int barX = x + 90;

    if (nosRatio > 0.0f) {
        painter.save();
        QRadialGradient glow(barX + NUM_SEGMENTS * (SEG_W + SEG_GAP) / 2,
            y + SEG_H / 2, 50);
        glow.setColorAt(0.0, QColor(255, 150, 0, 60));
        glow.setColorAt(1.0, Qt::transparent);
        painter.setBrush(glow);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(barX - 10, y - 10,
            NUM_SEGMENTS * (SEG_W + SEG_GAP) + 20, SEG_H + 20);
        painter.restore();
    }

    for (int i = 0; i < NUM_SEGMENTS; i++) {
        QRect seg(barX + i * (SEG_W + SEG_GAP), y, SEG_W, SEG_H);
        if (i < filledCount) {
            QLinearGradient grad(seg.topLeft(), seg.bottomRight());
            grad.setColorAt(0.0, QColor(255, 180, 0));
            grad.setColorAt(1.0, QColor(210, 70, 0));
            painter.setBrush(grad);
            painter.setPen(QColor(255, 210, 50, 200));
        }
        else {
            painter.setBrush(QColor(30, 20, 0, 150));
            painter.setPen(QColor(80, 55, 0, 120));
        }
        painter.drawRoundedRect(seg, CORNER, CORNER);
    }
}
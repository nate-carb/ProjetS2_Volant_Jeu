#include "mainwindownate.h"

#define NOMINMAX
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QDir>
#include "Vehicule.h"
#include <QKeyEvent>
#include <trackViewer.h>
#include <QPainterPath>
#include <windows.h>

const float PIXELS_PER_METER = 5.0f;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), imageX(100), imageY(100)
{
    // Voir où le programme cherche les fichiers
    qDebug() << "Dossier de travail actuel:" << QDir::currentPath();

	

    // Essayer de charger l'image
    image = QPixmap("images/car.PNG");  // Remplace par ton nom de fichier
	image = image.scaled(60, 60, Qt::KeepAspectRatio);

    raceTimes = new RaceTimes();
	//Vehicule* voiture = new Vehicule;
    voiture = Vehicule();
	//track = Track();
	//track.loadFromFile("tracks/defaultTrack1.trk");
    track = new Track();       // assigns to the MEMBER pointer
    //track->loadFromFile("tracks/segTest1.trk");
	track->loadFromFile("tracks/trackCheckpoint1.trk");
    //track->loadFromFile("tracks/track3dmodelV1.trk");
    //track->loadFromFile("tracks/test_pit.trk");
    // Vérifier si ça a marché
    pitStop.placeNearTrack(track->getCenterLine(), 5.0f, 150.0f, track->getTrackWidth());
   
    if (image.isNull()) {
        qDebug() << "ERREUR: Image non chargée!";
        qDebug() << "Le fichier existe?" << QFile::exists("images/car.PNG");
    }
    else {
        qDebug() << "SUCCESS! Taille:" << image.size();
    }

    // Timer qui change la m�t�o toutes les 10 secondes
    weatherTimer = new QTimer(this);
    connect(weatherTimer, &QTimer::timeout, this, &MainWindow::changeWeather);
    weatherTimer->start(10000);  // 10 secondes

    resize(800, 600);

    // Crée un timer
    timer = new QTimer(this);

    // CONNECTE le timer à ta fonction gameLoop
    connect(timer, &QTimer::timeout, this, &MainWindow::gameLoop);

    // DÉMARRE le timer - déclenche toutes les 16ms
    timer->start(8);  // 16 millisecondes ? 60 fois par seconde

    lastFrameTime = QTime::currentTime();
}

MainWindow::~MainWindow()
{
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        keyPressEvent(static_cast<QKeyEvent*>(event));
        return true;
    }
    if (event->type() == QEvent::KeyRelease) {
        keyReleaseEvent(static_cast<QKeyEvent*>(event));
        return true;
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::changeWeather()
{
    // Cycle SUNNY -> RAINY -> STORMY -> SUNNY
    if (currentWeather == Vehicule::SUNNY)
        currentWeather = Vehicule::RAINY;
    else if (currentWeather == Vehicule::RAINY)
        currentWeather = Vehicule::STORMY;
    else
        currentWeather = Vehicule::SUNNY;

    voiture.setWeather(currentWeather);
    qDebug() << "M�t�o chang�e !";
}

// Cette fonction dessine l'image
//void MainWindow::paintEvent(QPaintEvent* event)
//{
//    QPainter painter(this);
//    const float PIXELS_PER_METER = 5.0f;
//    float x = voiture.getPosition().x()*PIXELS_PER_METER;
//    float y = voiture.getPosition().y()*PIXELS_PER_METER;
//    float angle = voiture.getAngle();  // en radians
//
//    painter.translate(x, y);                 // va à la position de la voiture
//    painter.rotate(angle * 180.0 / M_PI);   // Qt veut des degrés
//
//    // Dessine l'image centrée sur (0,0)
//    painter.drawPixmap(-image.width() / 2,
//        -image.height() / 2,
//        image);
//}
void MainWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // ===== FOND =====
    painter.fillRect(rect(), QColor(34, 139, 34));  // Herbe verte

    const float PIXELS_PER_METER = 5.0f;

    // ===== CAMÉRA QUI SUIT LA VOITURE =====
    float carX = voiture.getPosition().x() * PIXELS_PER_METER;
    float carY = voiture.getPosition().y() * PIXELS_PER_METER;

    // Centrer la caméra sur la voiture
    painter.translate(width() / 2 - carX, height() / 2 - carY);

    // ===== DESSINER LA PISTE ===== 
    drawTrack(painter, PIXELS_PER_METER);

    // ===== PITLANE =====
    painter.setBrush(QColor(50, 50, 50));
    painter.setPen(Qt::NoPen);
    painter.drawPath(pitStop.getPitLanePath(PIXELS_PER_METER));

    // Rebords rouges/blancs sur les 4 c�t�s
    QPainterPath path = pitStop.getPitLanePath(PIXELS_PER_METER);
    QPolygonF poly = path.toFillPolygon();

    std::vector<QVector2D> edgeTop, edgeBot, edgeRight;

    for (int i = 0; i <= 10; i++) {
        float t = i / 10.0f;
        edgeTop.push_back(QVector2D(
            poly[0].x() + t * (poly[3].x() - poly[0].x()),
            poly[0].y() + t * (poly[3].y() - poly[0].y())
        ));
        edgeBot.push_back(QVector2D(
            poly[1].x() + t * (poly[2].x() - poly[1].x()),
            poly[1].y() + t * (poly[2].y() - poly[1].y())
        ));
        edgeRight.push_back(QVector2D(
            poly[3].x() + t * (poly[2].x() - poly[3].x()),
            poly[3].y() + t * (poly[2].y() - poly[3].y())
        ));
    }

    drawCurbs(painter, edgeTop, 1.0f, Qt::red);
    drawCurbs(painter, edgeBot, 1.0f, Qt::red);
    drawCurbs(painter, edgeRight, 1.0f, Qt::red);

    // ===== PIT STOP BOX (jaune, plus gros) =====
    painter.setBrush(Qt::NoBrush);  // transparent � l'int�rieur
    painter.setPen(QPen(Qt::white, 2));
    painter.drawRect(pitStop.getRect());
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 8, QFont::Bold));
    painter.drawText(pitStop.getRect(), Qt::AlignCenter, "PIT\nSTOP");

    // ===== DESSINER LA VOITURE =====
    painter.save();
    painter.translate(carX, carY);
    painter.rotate(voiture.getAngle() * 180.0 / M_PI);
    painter.drawPixmap(-image.width() / 2, -image.height() / 2, image);
    painter.restore();

    // ===== HUD (fixe � l'�cran) =====
    painter.resetTransform();
    painter.setClipRect(rect());  // <-- force le clipping sur toute la fen�tre
    painter.setClipping(true);

    // ===== PLUIE =====
    if (currentWeather == Vehicule::RAINY || currentWeather == Vehicule::STORMY) {
        int numDrops = (currentWeather == Vehicule::STORMY) ? 150 : 75;
        float penWidth = (currentWeather == Vehicule::STORMY) ? 2.5f : 1.5f;
        painter.setPen(QPen(QColor(150, 150, 255, 150), penWidth));
        srand(QTime::currentTime().msec());
        for (int i = 0; i < numDrops; i++) {
            int x = rand() % width();
            int y = rand() % height();
            int length = (currentWeather == Vehicule::STORMY) ? 20 : 12;
            painter.drawLine(x, y, x - 3, y + length);
        }
    }

    // ===== COULEUR M�T�O =====
    QString weatherText;
    QColor weatherColor;
    switch (currentWeather) {
    case Vehicule::RAINY:
        weatherText = "Pluie";
        weatherColor = QColor(100, 150, 255);
        break;
    case Vehicule::STORMY:
        weatherText = "Tempete";
        weatherColor = QColor(150, 100, 255);
        break;
    default:
        weatherText = "Ensoleille";
        weatherColor = QColor(255, 220, 0);
        break;
    }

    // ===== FOND HUD =====
    painter.setBrush(QColor(0, 0, 0, 150));
    painter.setPen(Qt::NoPen);
    painter.drawRect(10, 10, 170, 115);

    // ===== TEXTE HUD =====
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(20, 35, QString("Carburant: %1%").arg((int)voiture.getCarburant()));

    painter.setPen(QColor(0, 200, 255));
    painter.drawText(20, 60, QString("NOS: %1%").arg((int)voiture.getNos()));

    painter.setPen(weatherColor);
    painter.drawText(20, 85, QString("Meteo: %1").arg(weatherText));

    // Couleur selon l'usure
    float wear = voiture.getTireWear();
    QColor tireColor;
    if (wear > 60)       tireColor = Qt::green;
    else if (wear > 30)  tireColor = QColor(255, 165, 0);  // orange
    else                 tireColor = Qt::red;

    painter.setPen(tireColor);
    painter.drawText(20, 108, QString("Pneus: %1%").arg((int)wear));

    // ===== MESSAGE PIT STOP =====
    if (inPitStop) {
        painter.setPen(Qt::green);
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(20, 120, "PIT STOP - Appuyez sur Enter pour partir !");
    }
	drawMinimap(painter);
}

// Cette fonction capte les clics de souris
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    // Met à jour les coordonnées
    imageX = event->pos().x() - image.width() / 2;
    imageY = event->pos().y() - image.height() / 2;

    // Redemande à Qt de redessiner
    update();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_W) keyW = true;
    if (event->key() == Qt::Key_A) keyA = true;
    if (event->key() == Qt::Key_S) keyS = true;
    if (event->key() == Qt::Key_D) keyD = true;
    if (event->key() == Qt::Key_Space) keySpace = true;
    if (event->key() == Qt::Key_Return) keyEnter = true;
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_W) keyW = false;
    if (event->key() == Qt::Key_A) keyA = false;
    if (event->key() == Qt::Key_S) keyS = false;
    if (event->key() == Qt::Key_D) keyD = false;
    if (event->key() == Qt::Key_Space) keySpace = false;
    if (event->key() == Qt::Key_Return) keyEnter = false;
}



void MainWindow::gameLoop()
{

    // Lecture directe des touches Windows
    keyW = (GetAsyncKeyState('W') & 0x8000) != 0;
    keyS = (GetAsyncKeyState('S') & 0x8000) != 0;
    keyA = (GetAsyncKeyState('A') & 0x8000) != 0;
    keyD = (GetAsyncKeyState('D') & 0x8000) != 0;
    keySpace = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    keyEnter = (GetAsyncKeyState(VK_RETURN) & 0x8000) != 0;

    QTime currentTime = QTime::currentTime();
    int msElapsed = lastFrameTime.msecsTo(currentTime);  // Millisecondes écoulées
    deltaTime = msElapsed / 1000.0f;  // Convertit en secondes
    lastFrameTime = currentTime;  // Sauvegarde pour la prochaine frame

    voiture.setAccel(keyW ? 1.0f : 0.0f);
    voiture.setBreaking(keyS ? 1.0f : 0.0f);
    voiture.setBoosting(keySpace);

    if (keyA && !keyD) voiture.setSteering(-1.0f);
    else if (keyD && !keyA) voiture.setSteering(1.0f);
    else voiture.setSteering(0.0f);
    // Sur la piste OU dans la pitlane OU dans le pit stop
    const float PIXELS_PER_METER = 5.0f;
    int carXpx = (int)(voiture.getPosition().x() * PIXELS_PER_METER);
    int carYpx = (int)(voiture.getPosition().y() * PIXELS_PER_METER);

    bool onTrack = track->isVector2DOnTrack(voiture.getPosition());
    bool onPitLane = pitStop.getPitLanePath(PIXELS_PER_METER)
        .contains(QPointF(carXpx, carYpx));
    bool onPit = pitStop.getRect().contains(carXpx, carYpx);

    if (!raceTimes->isRaceStarted()) { raceTimes->setupRace(1, track);  raceTimes->startRace(); }// ONLY FOR TESTING MUST BE CHANGE FOR FINAL VERSION
    //Checkpoint Check
    raceTimes->checkForCheckpoint(track, voiture.getPosition());

    voiture.is_on_grass = !(onTrack || onPitLane || onPit);
    voiture.is_on_track = !voiture.is_on_grass;

    

    //M�canique de pitstop
    int carX = (int)(voiture.getPosition().x() * PIXELS_PER_METER);
    int carY = (int)(voiture.getPosition().y() * PIXELS_PER_METER);
    inPitStop = pitStop.contains(carX, carY);

    if (!inPitStop) pitStop.resetLeaving();

    if (inPitStop && !pitStop.isLeaving() && !keyEnter) {
        float carburant = voiture.getCarburant();
        float nos = voiture.getNos();
        float tireWear = voiture.getTireWear();
        pitStop.recharge(deltaTime, carburant, nos, tireWear);
        voiture.setCarburant(carburant);
        voiture.setNos(nos);
        voiture.setTireWear(tireWear);
        update();
        return;
    }

    if (inPitStop && keyEnter) pitStop.setLeaving(true);

    // ===== UPDATE PHYSIQUE =====
    voiture.update(deltaTime);

    update();
}

void MainWindow::drawTrack(QPainter& painter, float scale)
{
    // PISTE (gris foncé)
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(50, 50, 50));
    std::vector<QVector2D> left = track->getTrackEdges().left;
    std::vector<QVector2D> right = track->getTrackEdges().right;
    
    // Dessiner la piste comme un polygone
    QPolygonF trackPoly;
    for (const auto& p : left) {
        trackPoly << QPointF(p.x() * scale, p.y() * scale);
    }
    for (int i = right.size() - 1; i >= 0; i--) {
        trackPoly << QPointF(right[i].x() * scale, right[i].y() * scale);
    }
    painter.drawPolygon(trackPoly);

    // LIGNE CENTRALE (pointillés blancs)
    QPen centerPen(Qt::white, 2, Qt::DashLine);
    painter.setPen(centerPen);

    std::vector<QVector2D> center = track->getCenterLine();
    for (size_t i = 1; i < center.size(); i++) {
        painter.drawLine(
            QPointF(center[i - 1].x() * scale, center[i - 1].y() * scale),
            QPointF(center[i].x() * scale, center[i].y() * scale)
        );
    }

    // BORDURES (rouges et blanches alternées comme en F1)
    drawCurbs(painter, left, scale, Qt::red);
    drawCurbs(painter, right, scale, Qt::red);
	drawPit(scale, track, painter);

    // Draw sprites along center line
    auto centerLine = track->getCenterLine();
    auto pieces = track->getPieces();

    size_t pointIndex = 0; // tracks where we are in centerLine

    for (size_t i = 0; i < pieces.size(); i++) {
        if (!pieces[i]) { pointIndex++; continue; }  
        int segmentCount = pieces[i]->getLengths().size(); // how many points this piece uses

        size_t startIdx = pointIndex;
        size_t endIdx = pointIndex + segmentCount;

        if (endIdx >= centerLine.size()) break;

        // Use start and end of this piece for position and angle
        QVector2D startPos = centerLine[startIdx];
        QVector2D endPos = centerLine[endIdx];
        QVector2D midPos = (startPos + endPos) * 0.5f; // draw sprite at center of piece
        QVector2D dir = endPos - startPos;

        float angle = atan2(dir.y(), dir.x()) * 180.0f / M_PI + pieces[i]->getSpriteRotationOffset();

        QString path = pieces[i]->getSpritePath();
        QPixmap sprite(path);
        if (sprite.isNull()) { pointIndex += segmentCount; continue; }

        float spriteScale = (track->getTrackWidth() * scale) / sprite.width();

        painter.save();
        painter.translate(midPos.x() * scale, midPos.y() * scale);
        painter.rotate(angle);
        painter.scale(spriteScale, spriteScale);
        painter.drawPixmap(-sprite.width() / 2, -sprite.height() / 2, sprite);
        painter.restore();

        pointIndex += segmentCount;
    }
    
}

void MainWindow::drawCurbs(QPainter& painter, const std::vector<QVector2D>& edge, float scale, QColor color)
{
    QPen curbPen(color, 8);
    painter.setPen(curbPen);

    for (size_t i = 1; i < edge.size(); i++) {
        // Alterner rouge/blanc tous les 2 segments
        if ((i / 2) % 2 == 0) {
            painter.setPen(QPen(color, 8));
        }
        else {
            painter.setPen(QPen(Qt::white, 8));
        }

        painter.drawLine(
            QPointF(edge[i - 1].x() * scale, edge[i - 1].y() * scale),
            QPointF(edge[i].x() * scale, edge[i].y() * scale)
        );
    }
}

void MainWindow::drawPit(float scale, Track* track, QPainter& painter)
{
    // After drawCurbs calls, before sprite drawing:

// ── PIT LANE ──────────────────────────────────────────
    if (track->hasPitLane()) {
        PitLane pit = track->getPitLane();

        // Pit lane surface
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(80, 80, 80));

        QPolygonF pitPoly;
        for (const auto& p : pit.edges.left)
            pitPoly << QPointF(p.x() * scale, p.y() * scale);
        for (int i = pit.edges.right.size() - 1; i >= 0; i--)
            pitPoly << QPointF(pit.edges.right[i].x() * scale, pit.edges.right[i].y() * scale);
        painter.drawPolygon(pitPoly);

        // Entry curve surface
        if (!pit.entryCurveEdges.left.empty()) {
            QPolygonF entryPoly;
            for (const auto& p : pit.entryCurveEdges.left)
                entryPoly << QPointF(p.x() * scale, p.y() * scale);
            for (int i = pit.entryCurveEdges.right.size() - 1; i >= 0; i--)
                entryPoly << QPointF(pit.entryCurveEdges.right[i].x() * scale, pit.entryCurveEdges.right[i].y() * scale);
            painter.drawPolygon(entryPoly);
        }

        // Exit curve surface
        if (!pit.exitCurveEdges.left.empty()) {
            QPolygonF exitPoly;
            for (const auto& p : pit.exitCurveEdges.left)
                exitPoly << QPointF(p.x() * scale, p.y() * scale);
            for (int i = pit.exitCurveEdges.right.size() - 1; i >= 0; i--)
                exitPoly << QPointF(pit.exitCurveEdges.right[i].x() * scale, pit.exitCurveEdges.right[i].y() * scale);
            painter.drawPolygon(exitPoly);
        }

        // Pit center line (orange dashed)
        QPen pitCenterPen(QColor(255, 165, 0), 2, Qt::DashLine);
        painter.setPen(pitCenterPen);
        for (size_t i = 1; i < pit.centerLine.size(); i++)
            painter.drawLine(
                QPointF(pit.centerLine[i - 1].x() * scale, pit.centerLine[i - 1].y() * scale),
                QPointF(pit.centerLine[i].x() * scale, pit.centerLine[i].y() * scale));

        // Pit curbs
        drawCurbs(painter, pit.edges.left, scale, Qt::red);
        drawCurbs(painter, pit.edges.right, scale, Qt::red);
    }
}
    
void MainWindow::drawMinimap(QPainter& painter)
{
    if (!track || track->getCenterLine().empty()) return;

    // ── Dimensions et position de la mini-map ────────────────
    const int MAP_W = 180;
    const int MAP_H = 180;
    const int MARGIN = 15;
    QRectF minimapRect(
        width() - MAP_W - MARGIN,
        height() - MAP_H - MARGIN,
        MAP_W, MAP_H
    );

    // ── Calculer les bounds de la piste ──────────────────────
    float minX = 1e9f, maxX = -1e9f;
    float minY = 1e9f, maxY = -1e9f;

    auto checkPoint = [&](const QVector2D& p) {
        minX = std::min(minX, p.x()); maxX = std::max(maxX, p.x());
        minY = std::min(minY, p.y()); maxY = std::max(maxY, p.y());
        };
    for (const auto& p : track->getCenterLine())       checkPoint(p);
    for (const auto& p : track->getTrackEdges().left)  checkPoint(p);
    for (const auto& p : track->getTrackEdges().right) checkPoint(p);

    // Ajouter une marge autour de la piste
    float padding = (maxX - minX) * 0.05f;
    minX -= padding; maxX += padding;
    minY -= padding; maxY += padding;

    float rangeX = maxX - minX;
    float rangeY = maxY - minY;
    if (rangeX <= 0 || rangeY <= 0) return;

    // ── Fonction de projection monde → mini-map ──────────────
    auto toMinimap = [&](const QVector2D& p) -> QPointF {
        float nx = (p.x() - minX) / rangeX;
        float ny = (p.y() - minY) / rangeY;
        return QPointF(
            minimapRect.left() + nx * minimapRect.width(),
            minimapRect.top() + ny * minimapRect.height()
        );
        };

    // ── Fond semi-transparent ─────────────────────────────────
    painter.setBrush(QColor(0, 0, 0, 160));
    painter.setPen(QPen(QColor(255, 255, 255, 80), 1));
    painter.drawRoundedRect(minimapRect, 8, 8);

    // ── Clipping pour rester dans la mini-map ────────────────
    painter.save();
    QPainterPath clipPath;
    clipPath.addRoundedRect(minimapRect, 8, 8);
    painter.setClipPath(clipPath);

    // ── Dessiner la surface de la piste (polygone rempli) ────
    const auto& left = track->getTrackEdges().left;
    const auto& right = track->getTrackEdges().right;

    if (left.size() >= 2 && right.size() >= 2) {
        QPolygonF trackPoly;
        for (const auto& p : left)
            trackPoly << toMinimap(p);
        for (int i = (int)right.size() - 1; i >= 0; i--)
            trackPoly << toMinimap(right[i]);

        painter.setBrush(QColor(80, 80, 80, 220));
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(trackPoly);
    }

    // ── Dessiner la pit lane ──────────────────────────────────
    if (track->hasPitLane()) {
        PitLane pit = track->getPitLane();

        // Surface de la pit lane
        if (!pit.edges.left.empty() && !pit.edges.right.empty()) {
            QPolygonF pitPoly;
            for (const auto& p : pit.edges.left)
                pitPoly << toMinimap(p);
            for (int i = (int)pit.edges.right.size() - 1; i >= 0; i--)
                pitPoly << toMinimap(pit.edges.right[i]);

            painter.setBrush(QColor(100, 100, 120, 220));
            painter.setPen(Qt::NoPen);
            painter.drawPolygon(pitPoly);
        }

        // Courbes d'entrée/sortie
        auto drawCurveEdges = [&](const std::vector<QVector2D>& edgeL,
            const std::vector<QVector2D>& edgeR) {
                if (edgeL.empty() || edgeR.empty()) return;
                QPolygonF poly;
                for (const auto& p : edgeL)  poly << toMinimap(p);
                for (int i = (int)edgeR.size() - 1; i >= 0; i--)
                    poly << toMinimap(edgeR[i]);
                painter.setBrush(QColor(100, 100, 120, 220));
                painter.setPen(Qt::NoPen);
                painter.drawPolygon(poly);
            };
        drawCurveEdges(pit.entryCurveEdges.left, pit.entryCurveEdges.right);
        drawCurveEdges(pit.exitCurveEdges.left, pit.exitCurveEdges.right);
    }

    // ── Ligne centrale (pointillés blancs fins) ───────────────
    const auto& center = track->getCenterLine();
    painter.setPen(QPen(QColor(255, 255, 255, 80), 1, Qt::DashLine));
    // ===== POINT DU JOUEUR =====
    float nx = (voiture.getPosition().x() - minX) / (maxX - minX);
    float ny = (voiture.getPosition().y() - minY) / (maxY - minY);
    QPointF carMiniPos(
        minimapRect.left() + nx * minimapRect.width(),
        minimapRect.top() + ny * minimapRect.height()
    );
    painter.setBrush(Qt::red);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(carMiniPos, 3, 3);

}

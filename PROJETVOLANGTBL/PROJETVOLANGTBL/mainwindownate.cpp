#include "mainwindownate.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QDir>
#include "Vehicule.h"
#include <QKeyEvent>
#include <trackViewer.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), imageX(100), imageY(100)
{
    // Voir où le programme cherche les fichiers
    qDebug() << "Dossier de travail actuel:" << QDir::currentPath();
    // Essayer de charger l'image

    image = QPixmap("images/car.PNG");  // Remplace par ton nom de fichier
	image = image.scaled(60, 60, Qt::KeepAspectRatio);

	voiture = Vehicule();
    track = Track();
	track.loadFromFile("tracks/defaultTrack1.trk");
    // Vérifier si ça a marché
    if (image.isNull()) {
        qDebug() << "ERREUR: Image non chargée!";
        qDebug() << "Le fichier existe?" << QFile::exists("images/car.PNG");
    }
    else {
        qDebug() << "SUCCESS! Taille:" << image.size();
    }

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

    // ===== DESSINER LA VOITURE =====
    painter.save();
    painter.translate(carX, carY);
    painter.rotate(voiture.getAngle() * 180.0 / M_PI);
    painter.drawPixmap(-image.width() / 2, -image.height() / 2, image);
    painter.restore();
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
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_W) keyW = false;
    if (event->key() == Qt::Key_A) keyA = false;
    if (event->key() == Qt::Key_S) keyS = false;
    if (event->key() == Qt::Key_D) keyD = false;
}



void MainWindow::gameLoop()
{
    QTime currentTime = QTime::currentTime();
    int msElapsed = lastFrameTime.msecsTo(currentTime);  // Millisecondes écoulées
    deltaTime = msElapsed / 1000.0f;  // Convertit en secondes
    lastFrameTime = currentTime;  // Sauvegarde pour la prochaine frame

    voiture.setAccel(keyW ? 1.0f : 0.0f);
    voiture.setBreaking(keyS ? 1.0f : 0.0f);

    if (keyA && !keyD) voiture.setSteering(-1.0f);
    else if (keyD && !keyA) voiture.setSteering(1.0f);
    else voiture.setSteering(0.0f);
    if (track.isVector2DOnTrack(voiture.getPosition())) {
		voiture.is_on_grass = false;
		voiture.is_on_track = true;
    }
    else {
		voiture.is_on_grass = true;
		voiture.is_on_track = false;
    }
    // ===== UPDATE PHYSIQUE =====
    voiture.update(deltaTime);

    update();
}

void MainWindow::drawTrack(QPainter& painter, float scale)
{
    // PISTE (gris foncé)
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(50, 50, 50));

    std::vector<QVector2D> left = track.getTrackEdges().left;
    std::vector<QVector2D> right = track.getTrackEdges().right;

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

    std::vector<QVector2D> center = track.getCenterLine();
    for (size_t i = 1; i < center.size(); i++) {
        painter.drawLine(
            QPointF(center[i - 1].x() * scale, center[i - 1].y() * scale),
            QPointF(center[i].x() * scale, center[i].y() * scale)
        );
    }

    // BORDURES (rouges et blanches alternées comme en F1)
    drawCurbs(painter, left, scale, Qt::red);
    drawCurbs(painter, right, scale, Qt::red);
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
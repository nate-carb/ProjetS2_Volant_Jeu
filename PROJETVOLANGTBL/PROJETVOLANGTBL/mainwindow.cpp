#include "mainwindow.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QDir>
#include "Vehicule.h"
#include <QKeyEvent>


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), imageX(100), imageY(100)
{
    // Voir où le programme cherche les fichiers
    qDebug() << "Dossier de travail actuel:" << QDir::currentPath();
    // Essayer de charger l'image

    image = QPixmap("images/car.PNG");  // Remplace par ton nom de fichier
	image = image.scaled(100, 100, Qt::KeepAspectRatio);
	voiture = Vehicule();

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
void MainWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    const float PIXELS_PER_METER = 4.0f;
    float x = voiture.getPosition().x()*PIXELS_PER_METER;
    float y = voiture.getPosition().y()*PIXELS_PER_METER;
    float angle = voiture.getAngle();  // en radians

    painter.translate(x, y);                 // va à la position de la voiture
    painter.rotate(angle * 180.0 / M_PI);   // Qt veut des degrés

    // Dessine l'image centrée sur (0,0)
    painter.drawPixmap(-image.width() / 2,
        -image.height() / 2,
        image);
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

    // ===== UPDATE PHYSIQUE =====
    voiture.update(deltaTime);

    update();
}
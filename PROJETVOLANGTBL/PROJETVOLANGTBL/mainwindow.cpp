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
	image = image.scaled(200, 100, Qt::KeepAspectRatio);
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
	imageX = voiture.getPosition().x;
	imageY = voiture.getPosition().y;
    painter.drawPixmap(imageX, imageY, image);
    painter.drawText(100, 100, "Salut");
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
    // W - Monter
    if (event->key() == Qt::Key_W) {
        voiture.setAccel(1.0);
        voiture.update(0.016f);// Simule une frame de 16ms
        update();      // Redemande à Qt de redessiner
		voiture.setAccel(0.0); // Réinitialise l'accélération pour éviter une accélération continue
    }

    // A - Gauche
    else if (event->key() == Qt::Key_A) {
        voiture.setSteering(-1);
        voiture.update(0.016f);
		voiture.setSteering(0); // Réinitialise le steering pour éviter une rotation continue
        update();
    }

    // S - Descendre
    else if (event->key() == Qt::Key_S) {
		voiture.setBreaking(1.0);
        voiture.update(0.016f);
        update();
        voiture.setBreaking(1.0);
    }

    // D - Droite
    else if (event->key() == Qt::Key_D) {
        voiture.setSteering(1);
        voiture.update(0.016f);
        update();
		voiture.setSteering(0); // Réinitialise le steering pour éviter une rotation continue
    }

    // 0 - Réinitialiser position
    else if (event->key() == Qt::Key_0) {
        imageX = 100;
        imageY = 100;
        voiture.update(0.016f);
        update();
    }
}

void MainWindow::gameLoop()
{
    QTime currentTime = QTime::currentTime();
    int msElapsed = lastFrameTime.msecsTo(currentTime);  // Millisecondes écoulées
    deltaTime = msElapsed / 1000.0f;  // Convertit en secondes
    lastFrameTime = currentTime;  // Sauvegarde pour la prochaine frame

	voiture.update(deltaTime);  // Met à jour la voiture avec le delta time
     

    update();
}
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

    // Crée un pitstop
    pitStop = QRect(300, 250, 80, 80);   // position et taille qu'on peut ajuster
    inPitStop = false;
    pitStopReady = false;
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

    // Affiche le niveau de NOS a l'écran
    painter.resetTransform();   // important — annule le translate/rotate de la voiture (jsp si necessaire, j'avais fait le 
                                // boost initial mais Claude m'a dit de mettre ca en checkant ce que j'avais fait)
    // Texte carburant
    painter.setPen(Qt::white);      //blanc carburant
    painter.setFont(QFont("Arial", 12));
    painter.drawText(20, 30, QString("Carburant: %1%").arg((int)voiture.getCarburant()));

    // Texte NOS
    painter.setPen(QColor(0, 200, 255));        // bleu NOS
    painter.drawText(20, 55, QString("NOS: %1%").arg((int)voiture.getNos()));

    // Zone pit stop
    painter.setBrush(QColor(255, 200, 0, 180));   // jaune semi-transparent
    painter.setPen(QPen(Qt::yellow, 2));
    painter.drawRect(pitStop);
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(pitStop, Qt::AlignCenter, "PIT\nSTOP");

    // Indicateur si on est dedans
    if (inPitStop) {
        painter.setPen(Qt::green);
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        if (!pitStopReady) {
            painter.drawText(20, 90, "PIT STOP - Rechargement...");
        }
        else {
            painter.drawText(20, 90, "PRÊT ! Appuie sur Entrée pour partir !");
        }
    }
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

    // Détection de la zone de pit stop
    const float PIXELS_PER_METER = 4.0f;
    int carX = (int)(voiture.getPosition().x() * PIXELS_PER_METER);
    int carY = (int)(voiture.getPosition().y() * PIXELS_PER_METER);

    inPitStop = pitStop.contains(carX, carY);

    inPitStop = pitStop.contains(carX, carY);

    // Reset quand on a complètement quitté la zone
    if (!inPitStop) leavingPitStop = false;

    if (inPitStop && !leavingPitStop && !keyEnter) {
        const float rechargeRate = 20.0f;
        voiture.setCarburant(std::min(voiture.getCarburant() + rechargeRate * deltaTime, 100.0f));
        voiture.setNos(std::min(voiture.getNos() + rechargeRate * deltaTime, 100.0f));
        update();
        return;
    }

    // Joueur appuie Entrée — on note qu'il veut partir
    if (inPitStop && keyEnter) leavingPitStop = true;

    // ===== UPDATE PHYSIQUE =====
    voiture.update(deltaTime);

    update();
}
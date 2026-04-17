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
    qDebug() << "Dossier de travail actuel:" << QDir::currentPath();
    image = QPixmap("images/car.PNG");  
	image = image.scaled(100, 100, Qt::KeepAspectRatio);
	voiture = Vehicule();

    if (image.isNull()) {
        qDebug() << "ERREUR: Image non chargée!";
        qDebug() << "Le fichier existe?" << QFile::exists("images/car.PNG");
    }
    else {
        qDebug() << "SUCCESS! Taille:" << image.size();
    }

    resize(800, 600);

    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &MainWindow::gameLoop);

    timer->start(8);  

    lastFrameTime = QTime::currentTime();

    pitStop = QRect(300, 250, 80, 80);   
    inPitStop = false;
    pitStopReady = false;
}

MainWindow::~MainWindow()
{
}

void MainWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    const float PIXELS_PER_METER = 4.0f;
    float x = voiture.getPosition().x()*PIXELS_PER_METER;
    float y = voiture.getPosition().y()*PIXELS_PER_METER;
    float angle = voiture.getAngle(); 

    painter.translate(x, y);                
    painter.rotate(angle * 180.0 / M_PI);   

    painter.drawPixmap(-image.width() / 2,
        -image.height() / 2,
        image);

    painter.resetTransform();   
    painter.setPen(Qt::white);      
    painter.setFont(QFont("Arial", 12));
    painter.drawText(20, 30, QString("Carburant: %1%").arg((int)voiture.getCarburant()));

    painter.setPen(QColor(0, 200, 255));        
    painter.drawText(20, 55, QString("NOS: %1%").arg((int)voiture.getNos()));

    painter.setBrush(QColor(255, 200, 0, 180));  
    painter.setPen(QPen(Qt::yellow, 2));
    painter.drawRect(pitStop);
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(pitStop, Qt::AlignCenter, "PIT\nSTOP");

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

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    imageX = event->pos().x() - image.width() / 2;
    imageY = event->pos().y() - image.height() / 2;

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
    int msElapsed = lastFrameTime.msecsTo(currentTime); 
    deltaTime = msElapsed / 1000.0f;  
    lastFrameTime = currentTime;  

    voiture.setAccel(keyW ? 1.0f : 0.0f);
    voiture.setBreaking(keyS ? 1.0f : 0.0f);
    voiture.setBoosting(keySpace);

    if (keyA && !keyD) voiture.setSteering(-1.0f);
    else if (keyD && !keyA) voiture.setSteering(1.0f);
    else voiture.setSteering(0.0f);

    const float PIXELS_PER_METER = 4.0f;
    int carX = (int)(voiture.getPosition().x() * PIXELS_PER_METER);
    int carY = (int)(voiture.getPosition().y() * PIXELS_PER_METER);

    inPitStop = pitStop.contains(carX, carY);

    inPitStop = pitStop.contains(carX, carY);

    if (!inPitStop) leavingPitStop = false;

    if (inPitStop && !leavingPitStop && !keyEnter) {
        const float rechargeRate = 20.0f;
        voiture.setCarburant(std::min(voiture.getCarburant() + rechargeRate * deltaTime, 100.0f));
        voiture.setNos(std::min(voiture.getNos() + rechargeRate * deltaTime, 100.0f));
        update();
        return;
    }

    if (inPitStop && keyEnter) leavingPitStop = true;

    voiture.update(deltaTime);

    update();
}
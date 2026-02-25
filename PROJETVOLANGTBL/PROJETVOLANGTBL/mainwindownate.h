#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include "Vehicule.h"
#include <QElapsedTimer>
#include <QTimer>
#include <QTime>
#include <Track.h>
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

    void drawTrack(QPainter& painter, float scale);
    void drawCurbs(QPainter& painter, const std::vector<QVector2D>& edge, float scale, QColor color);

private slots:
    void gameLoop();

private:
    QPixmap image;  // Ton image PNG
    float imageX;     // Position X
    float imageY;     // Position Y
    Vehicule voiture; 
	Track track; // Piste de course
    float deltaTime;             // Temps en secondes depuis dernière frame
	QTimer* timer;          // Timer pour la boucle de jeu
    QTime lastFrameTime; //Temos deouis derniere frame
    bool keyW = false;
    bool keyA = false;
    bool keyS = false;
    bool keyD = false;
    bool keySpace = false;
    bool keyEnter = false;
    QRect pitStop;          // zone du pit stop
    bool inPitStop;         // est-ce-que lnotre char est dans le pit stop
    bool pitStopReady;      // recharge terminée, peut partir
    bool leavingPitStop = false;
};

#endif
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include "Vehicule.h"
#include <QElapsedTimer>
#include <QTimer>
#include <QTime>

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

private slots:
    void gameLoop();

private:
    QPixmap image;  // Ton image PNG
    float imageX;     // Position X
    float imageY;     // Position Y
    Vehicule voiture; 
            
    float deltaTime;             // Temps en secondes depuis dernière frame
	QTimer* timer;          // Timer pour la boucle de jeu
    QTime lastFrameTime; //Temos deouis derniere frame
};

#endif
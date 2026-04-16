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
	void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void gameLoop();

private:
    QPixmap image;  
    float imageX;     
    float imageY;     
    Vehicule voiture; 
            
    float deltaTime;            
	QTimer* timer;         
    QTime lastFrameTime; 
    bool keyW = false;
    bool keyA = false;
    bool keyS = false;
    bool keyD = false;
    bool keySpace = false;
    bool keyEnter = false;
    QRect pitStop;         
    bool inPitStop;        
    bool pitStopReady;     
    bool leavingPitStop = false;
};

#endif
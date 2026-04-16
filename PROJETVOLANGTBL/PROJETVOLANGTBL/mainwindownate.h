#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QElapsedTimer>
#include <QTimer>
#include <QTime>
#include <Track.h>
#include "PitStop.h"
#include <QPixmap>
#include <map>
#include <SoundManager.h>
#include "Vehicule.h"
#include "raceTimes.h"
#include "ArduinoManager.h"
#include "RaceStart.h" 

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    ArduinoManager* arduino = nullptr; // comm arduino
    QTimer* timer;
    Vehicule voiture;
    Track* track = nullptr;
    RaceTimes* raceTimes = nullptr;
    SoundManager* soundManager = nullptr;
    Vehicule::Weather currentWeather = Vehicule::SUNNY;
    QTimer* weatherTimer;  // pour changer la météo automatiquement
    bool eventFilter(QObject* obj, QEvent* event) override;
    PitStop pitStop;
    RaceStart* raceStart = nullptr;
	PitStop* getPitStop() { return &pitStop; }  
    bool isPaused = false;
    ArduinoManager* getArduino() const { return arduino; }
    

    void resetInputs() {
        keyW = keyA = keyS = keyD = false;
        keySpace = keyEnter = keyP = false;
        keyShiftUp = keyShiftDown = false;
        prevKeyE = prevKeyQ = prevKeyF1 = false;
        inPitStop = false;
        if (raceStart) raceStart->reset();
    }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

    void drawTrack(QPainter& painter, float scale);
    void drawCurbs(QPainter& painter, const std::vector<QVector2D>& edge, float scale, QColor color);
    

    void drawPit(float scale, Track* track, QPainter& painter);
    void drawMinimap(QPainter& painter);
    
    

private slots:
    void gameLoop();
    void changeWeather();

private:

    QPixmap image;  // Ton image PNG
    float imageX;     // Position X
    float imageY;     // Position Y
 //   Vehicule voiture; 
	//Track track; // Piste de course
    float deltaTime;             // Temps en secondes depuis dernière frame
	//QTimer* timer;          // Timer pour la boucle de jeu
    QTime lastFrameTime; //Temos deouis derniere frame
    bool keyW = false;
    bool keyA = false;
    bool keyS = false;
    bool keyD = false;
    bool keySpace = false;
    bool keyEnter = false;
    bool inPitStop;
    bool keyP = false;
    bool keyShiftUp = false; // E
    bool keyShiftDown = false; // Q
    // Dans le .h, section private:
    bool prevKeyE = false;
    bool prevKeyQ = false;
    bool prevKeyF1 = false;
};

#endif
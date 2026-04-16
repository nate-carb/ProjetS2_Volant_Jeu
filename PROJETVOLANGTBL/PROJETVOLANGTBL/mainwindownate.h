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
#include "MuonWeather.h"
#include "WheelRotationPeak.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    ArduinoManager* arduino = nullptr; // comm arduino
	WheelRotationPeak* rotPeak = nullptr; // pour les pics de rotation
    QTimer* timer;
    Vehicule voiture;
    Track* track = nullptr;
    RaceTimes* raceTimes = nullptr;
    SoundManager* soundManager = nullptr;
    Vehicule::Weather currentWeather = Vehicule::SUNNY;
    QTimer* weatherTimer;  // pour changer la météo automatiquement
    bool eventFilter(QObject* obj, QEvent* event) override;
    PitStop pitStop;
    RaceStart*    raceStart    = nullptr;
    MuonWeather*  muonWeather  = nullptr;
	PitStop* getPitStop() { return &pitStop; }  
    bool isPaused = false;
    ArduinoManager* getArduino() const { return arduino; }
    

    void resetInputs() {
        keyW = keyA = keyS = keyD = false;
        keySpace = keyEnter = keyP = false;
        keyShiftUp = keyShiftDown = false;
        prevKeyE = prevKeyQ = prevKeyF1 = false;
        inPitStop = false;
        prevMuonCount = 0;
        if (raceStart)    raceStart->reset();
        if (muonWeather)  muonWeather->reset();
        currentWeather = Vehicule::SUNNY;
        voiture.setWeather(Vehicule::SUNNY);
        weatherTimer->start(10000);   // relance le cycle auto pour la prochaine course
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
    void applyWeather(Vehicule::Weather w) {
        weatherTimer->stop();   // les muons prennent le contrôle, on arrête le cycle auto
        currentWeather = w;
        voiture.setWeather(w);
    }

private:

    QPixmap image;                  // PNG
    float imageX;                   // Position X
    float imageY;                   // Position Y
    float deltaTime;                // Temps en secondes depuis dernière frame
    QTime lastFrameTime;            //Temps deouis derniere frame
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
    int  prevMuonCount = 0;   // edge detection muon
};

#endif
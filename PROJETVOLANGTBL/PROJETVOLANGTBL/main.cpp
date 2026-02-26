// main.cpp
#include <QtWidgets/QApplication>
#include "Vehicule.h"
#include "Track.h"
#include <windows.h>
#include <iostream>
#include <QDebug>
#include <chrono>
#include <QTimer>
#include "mainwindownate.h"
#include "mainWindowCreator.h"
#include "mainWindowView.h"
bool isKeyPressed(int vkCode) {
    return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
}

//int main(int argc, char* argv[])
//{
//    
//    //AllocConsole();
//
//    //FILE* fp;
//    //freopen_s(&fp, "CONOUT$", "w", stdout);
//    //freopen_s(&fp, "CONOUT$", "w", stderr);
//    //freopen_s(&fp, "CONIN$", "r", stdin);
//
//    //std::cout << "=== F1 RACING GAME ===" << std::endl;
//
//    //Vehicule f1car;
//
//    //// Chronomètre pour delta time
//    //auto lastTime = std::chrono::steady_clock::now();
//    
//    /*
//    while (true) {
//        // ===== DELTA TIME =====
//        auto currentTime = std::chrono::steady_clock::now();
//        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
//        lastTime = currentTime;
//
//        // ===== INPUT =====
//        // Réinitialiser les inputs chaque frame
//        f1car.setAccel(0.0f);
//        f1car.setSteering(0.0f);
//		f1car.setBreaking(0.0f);
//        if (isKeyPressed('W')) f1car.setAccel(1.0f);
//        if (isKeyPressed('S')) f1car.setBreaking(1.0f);  // Marche arrière plus lente
//        if (isKeyPressed('A')) f1car.setSteering(-1.0f);
//        if (isKeyPressed('D')) f1car.setSteering(1.0f);
//        if (isKeyPressed(VK_ESCAPE)) break;
//
//        // ===== UPDATE =====
//        f1car.update(deltaTime);
//
//        // ===== AFFICHAGE =====
//        system("cls");
//        std::cout << "=== F1 RACING ===" << std::endl;
//        std::cout << std::endl;
//
//        std::cout << "Position: ("
//            << (int)f1car.getPosition().x << ", "
//            << (int)f1car.getPosition().y << ")" << std::endl;
//
//        std::cout << "Vitesse: "
//            << (int)(f1car.getSpeed() * 3.6f) << " km/h" << std::endl;
//
//        std::cout << "Angle: "
//            << (int)(f1car.getAngle() * 57.3f) << " deg" << std::endl;
//
//        std::cout << "Carburant: "
//            << (int)f1car.getCarburant() << "%" << std::endl;
//
//        std::cout << std::endl;
//        std::cout << "FPS: " << (int)(1.0f / deltaTime) << std::endl;
//        std::cout << std::endl;
//        std::cout << "[W] Accelerer  [S] Freiner  [A] Gauche  [D] Droite  [ESC] Quitter" << std::endl;
//
//        // 60 FPS (16ms par frame)
//        Sleep(16);
//    }
//	*/
//
//    //std::cout << "Jeu termine!" << std::endl;
//
//
//    QApplication app(argc, argv);
//    MainWindow window;
//    window.show();
//    return app.exec();
//
//}

//3d test
#include "track3DViewer.h"

// ── Simple input handler ─────────────────────────────────────
// You probably already have this elsewhere – just an example

class GameLoop : public QObject
{
    Q_OBJECT
public:
    GameLoop(Track3DViewer* viewer, Vehicule* car, MainWindow* gameWindow, QObject* parent = nullptr)
        : QObject(parent), m_viewer(viewer), m_car(car)
    {
        // Run game update at ~60 fps
        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &GameLoop::update);
        timer->start(16); // ~60 fps
		
    }

public slots:
    void update()
    {
        float deltaTime = 0.016f; // fixed step, replace with real elapsed time

        // ── Your existing 2D game logic runs here ────────────
        m_car->update(deltaTime);

        // ── Tell the 3D viewer where the car is now ──────────
        m_viewer->updateVehicule(m_car);
    }

    // Hook this up to your keyboard handler
    void onAccel(bool pressed) { m_car->setAccel(pressed ? 1.0f : 0.0f); }
    void onBrake(bool pressed) { m_car->setBreaking(pressed ? 1.0f : 0.0f); }
    void onLeft(bool pressed) { m_car->setSteering(pressed ? -1.0f : 0.0f); }
    void onRight(bool pressed) { m_car->setSteering(pressed ? 1.0f : 0.0f); }

private:
    Track3DViewer* m_viewer;
    Vehicule* m_car;
	MainWindow* m_gameWindow;
};

// ─────────────────────────────────────────────────────────────
/*
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 1) Create and load your track (exactly as you do now)
    Track* track = new Track();
    track->loadFromFile("tracks/defaultTrack1.trk");
    qDebug() << "Left edge points:" << track->getTrackEdges().left.size();
    // 2) Create your vehicle (exactly as you do now)
    Vehicule* car = new Vehicule(0.0f, 0.0f);

    // 3) Create the 3D viewer window
    Track3DViewer* viewer = new Track3DViewer();
    viewer->resize(1280, 720);
    viewer->setTitle("Racing Game 3D");

    // 4) Give the viewer the track – it builds the 3D mesh automatically
    viewer->setTrack(track);

    // 5) Enable first-person camera (follows the car)
    //    Pass false to get orbit camera (useful for debugging)
    viewer->setFirstPersonMode(false);

    viewer->show();

    // 6) Start game loop
    GameLoop* loop = new GameLoop(viewer, car);

    return app.exec();
}*/
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Create 3D viewer
    Track3DViewer* viewer = new Track3DViewer();
    viewer->resize(1280, 720);
    viewer->setTitle("Racing Game 3D");
    viewer->setFirstPersonMode(true);
    viewer->show();

    // Create main window (already has its own timer/gameloop)
    MainWindow* window = new MainWindow();
    qDebug() << "track pointer:" << window->track;
    qDebug() << "track centerLine size:" << (window->track ? window->track->getCenterLine().size() : -1);
    // Give viewer the track from mainwindow
    viewer->setTrack(window->track);

    // Hook 3D viewer update into MainWindow's existing timer`
    QObject::connect(window->timer, &QTimer::timeout, [=]() {
        viewer->updateVehicule(&window->voiture);
        });

    window->show();

	MainWindowCreator* creator = new MainWindowCreator();
	creator->show();


    return app.exec();
}
#include "main.moc"

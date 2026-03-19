// main.cpp
#include <QtWidgets/QApplication>
#include "Vehicule.h"
#include "Track.h"
#include <windows.h>
#include <iostream>
#include <QDebug>
#include <chrono>
#include <QTimer>
#include <QPluginLoader>
#include <QDir>
#include "mainwindownate.h"
#include "mainWindowCreator.h"
#include "mainWindowView.h"
#include <QStackedLayout>
#include "HUDOverlay.h"
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
    qputenv("QT3D_RENDERER", "opengl");
    QApplication app(argc, argv);

    MainWindow* window = new MainWindow();
    window->timer->start(16);  // force le timer même sans show()

    // ===== WRAPPER =====
    Track3DViewer* viewer = new Track3DViewer();
    viewer->setFirstPersonMode(true);

    QWidget* container = QWidget::createWindowContainer(viewer);
    container->setMinimumSize(1280, 720);
    container->resize(1280, 720);
    container->setWindowTitle("Racing Game 3D");

    // ===== HUD flottant par-dessus =====
    HUDOverlay* hud = new HUDOverlay();  // pas de parent !
    hud->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    hud->setAttribute(Qt::WA_TranslucentBackground);
    hud->resize(container->size());
    hud->move(15, 15);  // coin supérieur gauche
    hud->show();
    // Positionne le HUD au bon endroit au départ
    hud->move(container->mapToGlobal(QPoint(15, 15)));

    // Suit la fenêtre de jeu quand elle bouge
    QObject::connect(window->timer, &QTimer::timeout, [=]() {
        hud->move(container->mapToGlobal(QPoint(0, 0)));
        hud->resize(container->size());
        });
    hud->setAttribute(Qt::WA_ShowWithoutActivating);
    container->setFocus();

    // ===== TIMER =====
    QObject::connect(window->timer, &QTimer::timeout, [=]() {
        viewer->updateVehicule(&window->voiture);
        hud->updateData(
            window->voiture.getCarburant(),
            window->voiture.getNos(),
            window->voiture.getTireWear(),
            window->currentWeather
        );
        });

    viewer->setTrack(window->track);

    // ===== CLAVIER =====
    container->setFocusPolicy(Qt::StrongFocus);
    container->installEventFilter(window);

    container->show();

    MainWindowCreator* creator = new MainWindowCreator();
    creator->show();
    return app.exec();
}

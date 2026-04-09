// main.cpp

// -- Qt includes --
#include <QtWidgets/QApplication>
#include <QDebug>
#include <chrono>
#include <QTimer>
#include <QPluginLoader>
#include <QDir>
#include <QStackedLayout>
// -- includes --
#include <windows.h>
#include <iostream>
// -- Home includes -- 
#include "Vehicule.h"
#include "Track.h"
#include "mainwindownate.h"
#include "mainWindowCreator.h"
#include "mainWindowView.h"
#include "HUDOverlay.h"
#include "MenuWindow.h"
bool isKeyPressed(int vkCode) {
    return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
}

int main(int argc, char* argv[])
{
    qputenv("QT3D_RENDERER", "opengl");
    QApplication app(argc, argv);

    // ===== MENU =====
    MenuWindow* menu = new MenuWindow();
    menu->show();

    // ===== JEU (créé mais caché) =====
    MainWindow* window = new MainWindow();
    window->timer->start(16);

    Track3DViewer* viewer = new Track3DViewer();
    viewer->setFirstPersonMode(true);

    QWidget* container = QWidget::createWindowContainer(viewer);
    container->setMinimumSize(1280, 720);
    container->resize(1280, 720);
    container->setWindowTitle("Racing Game 3D");

    HUDOverlay* hud = new HUDOverlay();
    hud->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    hud->setAttribute(Qt::WA_TranslucentBackground);
    hud->setAttribute(Qt::WA_ShowWithoutActivating);

    // ===== QUAND PLAY EST CLIQUÉ =====
    QObject::connect(menu, &MenuWindow::playRequested, [=](int trackIndex) {
        menu->hide();

        QStringList trackFiles = {
            "tracks/track1.trk",
            "tracks/track2.trk",
            "tracks/defaultTrack1.trk"
        };
        if (window->track)
            window->track->loadFromFile(
                trackFiles[trackIndex].toStdString());

        viewer->setTrack(window->track);
        container->setFocusPolicy(Qt::StrongFocus);
        container->installEventFilter(window);
        container->show();
        container->setFocus();
    hud->show();
        });

    // ===== TIMER DU JEU =====
    QObject::connect(window->timer, &QTimer::timeout, [=]() {
        if (!container->isVisible()) return;

        viewer->updateVehicule(&window->voiture);
        hud->updateData(
            window->voiture.getCarburant(),
            window->voiture.getNos(),
            window->voiture.getTireWear(),
            window->currentWeather,
            window->voiture.getSpeed(),
			window->voiture.getRpm(),
			window->voiture.getMaxRpm(),
			window->raceTimes->getCurrentLap(),
			window->raceTimes->getTotalLaps(),
			window->raceTimes->getBestLapMs(),
			window->raceTimes->getCurrentLapMs(),
			window->raceTimes->getDeltaMs(),
			window->voiture.getCarburant() < 15.0f || window->voiture.getTireWear() < 15.0f,
			window->raceTimes
        );

        static QPoint lastPos;
        QPoint currentPos = container->mapToGlobal(QPoint(0, 0));
        if (currentPos != lastPos) {
            hud->move(currentPos);
            hud->resize(container->size());
            lastPos = currentPos;
        } 
    }); 
    
    viewer->setTrack(window->track);
    
    return app.exec();
}
#include "main.moc"
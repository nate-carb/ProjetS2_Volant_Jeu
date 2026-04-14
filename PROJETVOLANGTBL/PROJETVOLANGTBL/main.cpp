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
#include "RaceEndDialog.h"
#include "PauseDialog.h"
#include "DeathDialog.h"

bool isKeyPressed(int vkCode) {
    return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
}

int main(int argc, char* argv[])
{
    qputenv("QT3D_RENDERER", "opengl");
    QApplication app(argc, argv);

    // ===== JEU (créé mais caché) =====
    MainWindow* window = new MainWindow();

    // ===== MENU =====
    MenuWindow* menu = new MenuWindow(window->soundManager);
    menu->show();
	MainWindowCreator* creator = new MainWindowCreator();
	creator->show();
    
    //window->timer->start(16);
    
	// --- For 3D viewer ---
    //QWidget* container = nullptr;
    //Track3DViewer* viewer = nullptr;

    Track3DViewer* viewer = new Track3DViewer();
    viewer->setFirstPersonMode(true);

    QWidget* container = QWidget::createWindowContainer(viewer);
    container->setMinimumSize(1280, 720);
    container->resize(1280, 720);
    container->setWindowTitle("Racing Game 3D");
    container->installEventFilter(viewer);

    HUDOverlay* hud = new HUDOverlay();
    hud->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    hud->setAttribute(Qt::WA_TranslucentBackground);
    hud->setAttribute(Qt::WA_ShowWithoutActivating);

    // ===== QUAND PLAY EST CLIQUÉ =====
    QObject::connect(menu, &MenuWindow::playRequested, [=](int trackIndex) {
        menu->hide();
        
        
		
        /*QStringList trackFiles = {
            "tracks/nate.trk",
            "tracks/nate2.trk",
            "tracks/defaultTrack1.trk"
        };*/
        QStringList trackNames = {
            "nate",
            "nate2",
            "TEST1"
        };
        //qDebug() << "Working directory:" << QDir::currentPath();
        //qDebug() << "Track name:" << trackNames[trackIndex];
       
        window->track->playTrack(trackNames[trackIndex]);  // uses the name from the menu
        window->pitStop.placePitLane(window->track->getPitLane(), window->track->getTrackWidth());
        

        
        //if (window->track) {
			//bool trackState = window->track->playTrack("nate2");
		 //   qDebug() << "Track play result:" << trackState;
   //         window->pitStop.placePitLane(window->track->getPitLane(), window->track->getTrackWidth());
            //window->track->loadFromFile(
            //    trackFiles[trackIndex].toStdString());
        //}
        // Create 3D viewer AFTER track is loaded
        //Track3DViewer* viewer = new Track3DViewer();
        //viewer->setFirstPersonMode(true);
        viewer->setTrack(window->track);

        //QWidget* container = QWidget::createWindowContainer(viewer);
        //container->setMinimumSize(1280, 720);
        //container->resize(1280, 720);
        //container->setWindowTitle("Racing Game 3D");
        container->setFocusPolicy(Qt::StrongFocus);
        container->installEventFilter(window);
        container->showFullScreen();
        container->setFocus();
        hud->show();

        window->timer->start(16);  // start game loop AFTER everything is ready
        });

    //QObject::connect(window->timer, &QTimer::timeout, [=, &container, &viewer]() {
    QObject::connect(window->timer, &QTimer::timeout, [=]() {
        if (!container || !container->isVisible() || container->isMinimized()) {
            hud->hide();
            return;
        }
        
        if (!window->track || window->track->getCenterLine().empty()) return;
        // Menu pause
        static bool pauseDialogOpen = false;
        if (window->isPaused && !pauseDialogOpen) {
            hud->setPaused(true);  // cache la pluie
            pauseDialogOpen = true;
            PauseDialog* dlg = new PauseDialog(window->soundManager, container);
            dlg->exec();

            PauseDialog::Result result = dlg->getResult();

            if (result == PauseDialog::RESUME) {
                window->isPaused = false;
            }
            else if (result == PauseDialog::RESTART) {
                window->isPaused = false;
                window->voiture = Vehicule();
                window->raceTimes->resetRace();
            }
            else if (result == PauseDialog::MAIN_MENU) {
                window->isPaused = false;
                container->hide();
                hud->hide();
                menu->goToMainMenu();
                menu->show();
                window->voiture = Vehicule();
                window->raceTimes->resetRace();
            }

            delete dlg;
            pauseDialogOpen = false;
            hud->setPaused(false);  // remet la pluie
            return;
        }

        hud->show();

        // Death screen
        static bool deathShown = false;
        if (!deathShown) {
            QString reason = "";
            if (window->voiture.getCarburant() <= 0.0f)
                reason = "Panne de carburant !";
            else if (window->voiture.getTireWear() <= 0.0f)
                reason = "Pneus totalement usés !";

            if (!reason.isEmpty()) {
                deathShown = true;
                window->isPaused = true;
                DeathDialog* dlg = new DeathDialog(reason, container);
                dlg->exec();

                DeathDialog::Result result = dlg->getResult();
                if (result == DeathDialog::RESTART) {
                    window->isPaused = false;
                    window->voiture = Vehicule();
                    window->raceTimes->resetRace();
                }
                else {
                    window->isPaused = false;
                    container->hide();
                    hud->hide();
                    menu->goToMainMenu();
                    menu->show();
                    window->voiture = Vehicule();
                    window->raceTimes->resetRace();
                }

                delete dlg;
                deathShown = false;
            }
        }

        viewer->updateVehicule(&window->voiture);

        // Détecte fin de course
        static bool raceEndShown = false;
        if (!raceEndShown && window->raceTimes->isRaceFinished()) {
            raceEndShown = true;
            int trackIndex = 0;
            int bestTime = (int)window->raceTimes->getTotalRaceTimeMs();
            RaceEndDialog* dlg = new RaceEndDialog(trackIndex, bestTime, container);
            dlg->exec();

            // Retour au menu principal
            container->hide();
            hud->hide();
            menu->show();
            window->voiture = Vehicule();
            window->raceTimes->resetRace();

            raceEndShown = false;
        }

        hud->updateData(
            window->voiture.getCarburant(),
            window->voiture.getNos(),
            window->voiture.getTireWear(),
            window->currentWeather,
			window->voiture.getSpeed(),     // m/s → km/h (ajuste si déjà en km/h)
			window->voiture.getRpm(),
			window->voiture.getMaxRpm(),
			window->raceTimes->getCurrentLap(),
			window->raceTimes->getTotalLaps(),
			window->raceTimes->getBestLapMs(),
			window->raceTimes->getCurrentLapMs(),
			window->raceTimes->getDeltaMs(),
			window->voiture.getCarburant() < 15.0f || window->voiture.getTireWear() < 15.0f,
			window->raceTimes,
			window->track,
			window->getPitStop(),
			window->voiture.getPosition(),
			window->voiture.getAngle()


        );

        hud->move(container->mapToGlobal(QPoint(0, 0)));
        hud->resize(container->size());
    });
    //viewer->setTrack(window->track);

    return app.exec();
}
    
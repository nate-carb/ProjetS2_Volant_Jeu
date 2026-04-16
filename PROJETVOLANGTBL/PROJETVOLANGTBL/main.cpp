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
	// ===== Connexion Arduino après 4 secondes =====
	QTimer::singleShot(2000, window, [window]() {   // ← attente 4 secondes pour laisser le temps au arduino de resetter et se préparer
        bool baseOk = window->arduino->connectBase("\\\\.\\COM4"); 
        bool wheelOk = window->arduino->connectWheel("\\\\.\\COM7");
        qDebug() << "Base connectee:" << baseOk;
        qDebug() << "Wheel connectee:" << wheelOk;
        });
    

    // ===== MENU =====
    MenuWindow* menu = new MenuWindow(window->soundManager, window->arduino);
    menu->show();
	// ===== TRACK CREATOR =====
	MainWindowCreator* creator = new MainWindowCreator();
	creator->show();
	// ===== 3D VIEWER =====
    Track3DViewer* viewer = new Track3DViewer();
	viewer->setFirstPersonMode(true); // view on the car, not above it

	// ===== CONTAINER (intègre le viewer dans une fenêtre Qt classique) =====
    QWidget* container = QWidget::createWindowContainer(viewer);
	container->setMinimumSize(1280, 720); 
	container->resize(1280, 720); 
	container->setWindowTitle("Racing Game 3D"); 
	container->installEventFilter(viewer); // pour que le viewer puisse recevoir les événements clavier même quand il est dans le container

	// ===== HUD (overlay transparent au-dessus du viewer) =====
    HUDOverlay* hud = new HUDOverlay();
    hud->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    hud->setAttribute(Qt::WA_TranslucentBackground);
    hud->setAttribute(Qt::WA_ShowWithoutActivating);

    auto resetGame = [=]() {
        // ── 1. Arrêter les timers ─────────────────────────────
        window->timer->stop();
        if (window->weatherTimer) window->weatherTimer->stop();
        window->resetInputs();


        // ── 2. Cacher le rendu et le HUD ──────────────────────
        container->hide();
        hud->hide();

        // ── 3. Reset état voiture et course ───────────────────
        window->voiture = Vehicule();
        window->raceTimes->resetRace();
        window->isPaused = false;
        window->currentWeather = Vehicule::SUNNY;

        // ── 4. Reset pit stop ─────────────────────────────────
        window->pitStop = PitStop();   // ou window->pitStop.reset() si la méthode existe

        // ── 5. Retour au menu ─────────────────────────────────
        menu->goToMainMenu();
        menu->show();
        };



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
            "NATE3"
        };
        //qDebug() << "Working directory:" << QDir::currentPath();
        //qDebug() << "Track name:" << trackNames[trackIndex];
       
        window->track->playTrack(trackNames[trackIndex]);  // uses the name from the menu
        window->pitStop.placePitLane(window->track->getPitLane(), window->track->getTrackWidth());

        // Spawn the car at the start of the track with the correct heading
        {
            float startAngleDeg = window->track->getStartAngle();
            window->voiture.setAngle(qDegreesToRadians(startAngleDeg));

            auto cl = window->track->getCenterLine();
            if (!cl.empty()) {
                window->voiture.setPosition(cl[0].x(), cl[0].y());
            }
        }
        window->rotPeak->resetPeaks();


        viewer->setTrack(window->track);


        container->setFocusPolicy(Qt::StrongFocus);
        container->installEventFilter(window);
        container->show();
        container->setFocus();
        hud->show();

        //// TEST ONLY POUR LA FIN DE COURSE
        //static bool testDone = false;
        //if (!testDone) {
        //    testDone = true;
        //    window->raceTimes->forceFinish();
        //}

		//window->arduino->update(); // update Arduino state once before starting the game loop - flush data
        window->timer->start(16);  // start game loop AFTER everything is ready
        });

    
    QObject::connect(window->timer, &QTimer::timeout, [=]() {
        bool raceEndOpen = false;
        for (QWidget* w : QApplication::allWidgets()) {
            if (w->objectName() == "RaceEndDialog" && w->isVisible()) {
                raceEndOpen = true;
                break;
            }
        }
        if (raceEndOpen) return;
        if (!container || !container->isVisible() || container->isMinimized()) {
            hud->hide();
            return;
        }

        if (window->arduino->getWheelData().switchBL && !window->arduino->prevbl) {
            viewer->changeCameraMode();
		}
        window->arduino->prevbl = window->arduino->getWheelData().switchBL;

        
        if (!window->track || window->track->getCenterLine().empty()) return;

        // ===== NAVIGATION JOYSTICK =====
        static int lastJoyDir = 0;
        static bool lastBtnSelect = false;

        int joyDir = window->arduino->getWheelData().joyDir;
        bool btnSelect = window->arduino->getWheelData().switchBR;

        if (joyDir != lastJoyDir && joyDir != 0) {
            Qt::Key key;
            switch (joyDir) {
            case 1: key = Qt::Key_Up;    break;
            case 2: key = Qt::Key_Down;  break;
            case 3: key = Qt::Key_Left;  break;
            case 4: key = Qt::Key_Right; break;
            default: key = Qt::Key_Down; break;
            }

            // Envoie au menu si visible, sinon au dialog actif
            QWidget* target = nullptr;
            if (menu->isVisible()) {
                QWidget* page = menu->getCurrentPage();
                QList<QPushButton*> buttons = page->findChildren<QPushButton*>();
                for (auto* btn : buttons) {
                    if (btn->hasFocus()) { target = btn; break; }
                }
                if (!target && !buttons.isEmpty()) {
                    buttons.first()->setFocus();
                    target = buttons.first();
                }
            }
            else {
                // Pour pause/raceend dialogs
                QWidget* activeWindow = QApplication::activeWindow();
                if (activeWindow) {
                    QList<QPushButton*> buttons = activeWindow->findChildren<QPushButton*>();
                    for (auto* btn : buttons) {
                        if (btn->hasFocus()) { target = btn; break; }
                    }
                    if (!target && !buttons.isEmpty()) {
                        buttons.first()->setFocus();
                        target = buttons.first();
                    }
                }
            }

            if (target) {
                QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier);
                QApplication::sendEvent(target->parentWidget(), keyEvent);
                delete keyEvent;
            }
        }
        lastJoyDir = joyDir;

       

        // Sélection
        if (btnSelect && !lastBtnSelect) {
            QWidget* focused = QApplication::focusWidget();
            if (focused) {
                QPushButton* btn = qobject_cast<QPushButton*>(focused);
                if (btn) btn->click();
            }
        }
        lastBtnSelect = btnSelect;

        // Menu pause
        static bool pauseDialogOpen = false;
        if (window->isPaused && !pauseDialogOpen) {
            hud->setPaused(true);  // cache la pluie
            pauseDialogOpen = true;
            window->timer->stop();
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
                
                resetGame();
            }

            delete dlg;
            pauseDialogOpen = false;
            
            hud->setPaused(false);  // remet la pluie
            if (!window->isPaused) window->timer->start(16);
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
                window->timer->stop();
                DeathDialog* dlg = new DeathDialog(reason, container);
                dlg->exec();

                DeathDialog::Result result = dlg->getResult();
                if (result == DeathDialog::RESTART) {
                    window->isPaused = false;
                    window->voiture = Vehicule();
                    window->raceTimes->resetRace();
                    
                }
                else {
                    resetGame();
                   
                }

                delete dlg;
                deathShown = false;
                if (container->isVisible()) window->timer->start(16);
            }
        }

        viewer->updateVehicule(&window->voiture);

        // Détecte fin de course
        static bool raceEndShown = false;
        if (!raceEndShown && window->raceTimes->isRaceFinished()) {
            raceEndShown = true;
            window->timer->stop();
            int trackIndex = 0;
            int bestTime = (int)window->raceTimes->getTotalRaceTimeMs();
            RaceEndDialog* dlg = new RaceEndDialog(trackIndex, bestTime, container);
            dlg->exec();

            
            resetGame();

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
        hud->updateRaceStart(window->raceStart->getState(),   
            window->raceStart->getLightsOn());

        hud->move(container->mapToGlobal(QPoint(0, 0)));
        hud->resize(container->size());
    });
    // Timer séparé pour le joystick menu — tourne toujours
    QTimer* joystickTimer = new QTimer();
    joystickTimer->setInterval(100);
    QObject::connect(joystickTimer, &QTimer::timeout, [=]() {
        static int lastJoyDir = 0;
        static bool lastBtnSelect = false;

        int joyDir = window->arduino->getWheelData().joyDir;
        bool btnSelect = window->arduino->getWheelData().switchBR;

        if (joyDir != lastJoyDir && joyDir != 0) {
            Qt::Key key;
            switch (joyDir) {
            case 1: key = Qt::Key_Up;    break;
            case 2: key = Qt::Key_Down;  break;
            case 3: key = Qt::Key_Right;  break;
            case 4: key = Qt::Key_Left; break;
            default: key = Qt::Key_Down; break;
            }

            QPushButton* focused = nullptr;

            if (menu->isVisible()) {
                QWidget* page = menu->getCurrentPage();
                QList<QPushButton*> buttons = page->findChildren<QPushButton*>();
                for (auto* btn : buttons) {
                    if (btn->hasFocus()) { focused = btn; break; }
                }
                if (!focused && !buttons.isEmpty()) {
                    buttons.first()->setFocus();
                    focused = buttons.first();
                }
            }
            else {
                for (QWidget* w : QApplication::topLevelWidgets()) {
                    QDialog* dlg = qobject_cast<QDialog*>(w);
                    if (dlg && dlg->isVisible()) {
                        QList<QPushButton*> buttons = dlg->findChildren<QPushButton*>();
                        for (auto* btn : buttons) {
                            if (btn->hasFocus()) { focused = btn; break; }
                        }
                        if (!focused && !buttons.isEmpty()) {
                            buttons.first()->setFocus();
                            focused = buttons.first();
                        }
                        break;
                    }
                }
            }

            if (focused) {
                QKeyEvent* ev = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier);
                QApplication::sendEvent(focused, ev);
                delete ev;
            }
        }
        lastJoyDir = joyDir;

        if (btnSelect && !lastBtnSelect) {
            qDebug() << "BTN SELECT PRESSED";
            QWidget* focused = QApplication::focusWidget();
            qDebug() << "Focused widget:" << focused;
            if (focused) {
                QPushButton* btn = qobject_cast<QPushButton*>(focused);
                qDebug() << "Button:" << btn;
                if (btn) btn->click();
            }
        }
        lastBtnSelect = btnSelect;

        // Encodeur 1 = volume
        static int lastEnc1 = -1;
        int enc1 = window->arduino->getWheelData().enc1;

        if (lastEnc1 == -1) {
            lastEnc1 = enc1;
        }

        int enc1Delta = enc1 - lastEnc1;
        lastEnc1 = enc1;

        if (enc1Delta != 0) {
            float step = 0.02f;
            float newVol = window->soundManager->getVolume() + enc1Delta * step;
            newVol = std::clamp(newVol, 0.0f, 1.0f);
            window->soundManager->setVolume(newVol);
        }

        //Encodeur2
        static int lastEnc2 = -1;
        int enc2 = window->arduino->getWheelData().enc2;

        if (lastEnc2 == -1) {
            lastEnc2 = enc2;  // initialise sans scroller
        }

        int enc2Delta = enc2 - lastEnc2;
        lastEnc2 = enc2;  // update TOUJOURS
        if (enc2Delta != 0) {
            QScrollArea* scrollArea = nullptr;

            if (menu->isVisible()) {
                QList<QScrollArea*> scrollAreas = menu->findChildren<QScrollArea*>();
                for (QScrollArea* sa : scrollAreas) {
                    if (sa->isVisible()) {
                        scrollArea = sa;
                        break;
                    }
                }
                qDebug() << "Visible ScrollArea:" << scrollArea;
            }
            if (scrollArea) {
                QScrollBar* bar = scrollArea->verticalScrollBar();
                qDebug() << "ScrollBar min:" << bar->minimum() << "max:" << bar->maximum() << "current:" << bar->value();
                bar->setValue(bar->value() + enc2Delta * 5);
            }

            for (QWidget* w : QApplication::topLevelWidgets()) {
                if (w->isVisible()) {
                    QScrollArea* sa = w->findChild<QScrollArea*>();
                    if (sa) { scrollArea = sa; break; }
                }
            }

            if (scrollArea) {
                QScrollBar* bar = scrollArea->verticalScrollBar();
                bar->setValue(bar->value() + enc2Delta * 5);
                qDebug() << "Scrolling by" << enc2Delta;
            }
        }
        });
    
    joystickTimer->start();

    return app.exec();
}
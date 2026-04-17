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
// -- Nos includes -- 
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

    
    MainWindow* window = new MainWindow();
	
	QTimer::singleShot(2000, window, [window]() {   // attente 4 secondes pour laisser le temps au arduino de resetter et se préparer
        bool baseOk = window->arduino->connectBase("\\\\.\\COM3"); 
        bool wheelOk = window->arduino->connectWheel("\\\\.\\COM6");
        qDebug() << "Base connectee:" << baseOk;
        qDebug() << "Wheel connectee:" << wheelOk;
        });
    

    // Menu
    MenuWindow* menu = new MenuWindow(window->soundManager, window->arduino);
    menu->show();
	// Track Creator
	MainWindowCreator* creator = new MainWindowCreator();
	creator->show();
	// 3D Viewer
    Track3DViewer* viewer = new Track3DViewer();
	viewer->setFirstPersonMode(true); 

	// Container
    QWidget* container = QWidget::createWindowContainer(viewer);
	container->setMinimumSize(1280, 720); 
	container->resize(1280, 720); 
	container->setWindowTitle("Racing Game 3D"); 
	container->installEventFilter(viewer); // pour que le joueur puisse recevoir les événements clavier même quand il est dans le container

	// HUD
    HUDOverlay* hud = new HUDOverlay();
    hud->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    hud->setAttribute(Qt::WA_TranslucentBackground);
    hud->setAttribute(Qt::WA_ShowWithoutActivating);

    auto resetGame = [=]() {
       
        window->timer->stop();
        if (window->weatherTimer) window->weatherTimer->stop();
        window->resetInputs();

        container->hide();
        hud->hide();

        window->voiture = Vehicule();
        window->raceTimes->resetRace();
        window->isPaused = false;
        window->currentWeather = Vehicule::SUNNY;

        window->pitStop = PitStop();  

        menu->goToMainMenu();
        menu->show();
        };

    // Play Button
    QObject::connect(menu, &MenuWindow::playRequested, [=](int trackIndex) {
        menu->hide();
     
        QStringList trackNames = {
            "nate",
            "nate2",
            "NATE3"
        };
       
        window->track->playTrack(trackNames[trackIndex]); 
        window->pitStop.placePitLane(window->track->getPitLane(), window->track->getTrackWidth());

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
        container->showFullScreen();
        container->setFocus();
        container->activateWindow();
        container->raise();
        hud->show();

        //// TEST ONLY POUR LA FIN DE COURSE
        //static bool testDone = false;
        //if (!testDone) {
        //    testDone = true;
        //    window->raceTimes->forceFinish();
        //}

        window->timer->start(16);  
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

        // Joystick
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
                // Pour pause et race end 
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
            RaceEndDialog* dlg = new RaceEndDialog(trackIndex, bestTime,
                window->rotPeak ? window->rotPeak->getPeakRotAccel() : 0.0f, container);
            dlg->exec();

            
            resetGame();

            raceEndShown = false;
        }

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

    // Timer séparé pour le joystick menu 
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

        // Encodeur 2 = tout ce qui est scrollable
        static int lastEnc2 = -1;
        int enc2 = window->arduino->getWheelData().enc2;

        if (lastEnc2 == -1) {
            lastEnc2 = enc2;  // initialise sans scroller
        }

        int enc2Delta = enc2 - lastEnc2;
        lastEnc2 = enc2;  // update tjrs
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
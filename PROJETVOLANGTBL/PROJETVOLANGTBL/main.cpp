#include "PROJETVOLANGTBL.h"
#include <QtWidgets/QApplication>
#include "Vehicule.h"
#include <windows.h>
#include <iostream>
#include <QDebug>

bool isKeyPressed(int vkCode) {
    return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
}
using namespace std;

int main(int argc, char* argv[])
{
    AllocConsole();

    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    std::cout << "Console OK\n";


	Vehicule f1car;
    f1car.setAccel(0.5f);

    while (true) {
        
        if (isKeyPressed('W')) f1car.setAccel(1);
		if (isKeyPressed('S')) f1car.setAccel(-1);
		if (isKeyPressed('A')) f1car.setSteering(-1);
        if (isKeyPressed('D')) f1car.setSteering(1);
        if (isKeyPressed(VK_ESCAPE)) break; // Quitter
		f1car.deplacement();
        qDebug() << "Position de la voiture : (" << f1car.getPosition().x << ", " << f1car.getPosition().y << ")";
		qDebug() << "Vitesse de la voiture : (" << f1car.getVitesse().x << ", " << f1car.getVitesse().y << ")";
		Sleep(1000); // Attendre un peu pour limiter la fréquence de mise à jour
    }


    QApplication app(argc, argv);
    PROJETVOLANGTBL window;
    window.show();
    return app.exec();
}

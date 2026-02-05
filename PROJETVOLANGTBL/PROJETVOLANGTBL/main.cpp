// main.cpp
#include "PROJETVOLANGTBL.h"
#include <QtWidgets/QApplication>
#include "Vehicule.h"
#include <windows.h>
#include <iostream>
#include <QDebug>
#include <chrono>

bool isKeyPressed(int vkCode) {
    return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
}

int main(int argc, char* argv[])
{
    AllocConsole();

    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    std::cout << "=== F1 RACING GAME ===" << std::endl;

    Vehicule f1car;

    // Chronomètre pour delta time
    auto lastTime = std::chrono::steady_clock::now();

    while (true) {
        // ===== DELTA TIME =====
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // ===== INPUT =====
        // Réinitialiser les inputs chaque frame
        f1car.setAccel(0.0f);
        f1car.setSteering(0.0f);
		f1car.setBreaking(0.0f);
        if (isKeyPressed('W')) f1car.setAccel(1.0f);
        if (isKeyPressed('S')) f1car.setBreaking(1.0f);  // Marche arrière plus lente
        if (isKeyPressed('A')) f1car.setSteering(-1.0f);
        if (isKeyPressed('D')) f1car.setSteering(1.0f);
        if (isKeyPressed(VK_ESCAPE)) break;

        // ===== UPDATE =====
        f1car.update(deltaTime);

        // ===== AFFICHAGE =====
        system("cls");
        std::cout << "=== F1 RACING ===" << std::endl;
        std::cout << std::endl;

        std::cout << "Position: ("
            << (int)f1car.getPosition().x << ", "
            << (int)f1car.getPosition().y << ")" << std::endl;

        std::cout << "Vitesse: "
            << (int)(f1car.getSpeed() * 3.6f) << " km/h" << std::endl;

        std::cout << "Angle: "
            << (int)(f1car.getAngle() * 57.3f) << " deg" << std::endl;

        std::cout << "Carburant: "
            << (int)f1car.getCarburant() << "%" << std::endl;

        std::cout << std::endl;
        std::cout << "FPS: " << (int)(1.0f / deltaTime) << std::endl;
        std::cout << std::endl;
        std::cout << "[W] Accelerer  [S] Freiner  [A] Gauche  [D] Droite  [ESC] Quitter" << std::endl;

        // 60 FPS (16ms par frame)
        Sleep(16);
    }

    std::cout << "Jeu termine!" << std::endl;

    // QApplication app(argc, argv);
    // PROJETVOLANGTBL window;
    // window.show();
    // return app.exec();

    return 0;
}
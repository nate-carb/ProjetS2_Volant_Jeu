#include "PROJETVOLANGTBL.h"
#include "map2d.h"
#include <QtWidgets/QApplication>
#include <windows.h>
#include <iostream>
#include <QDebug>
#include <chrono>

int main(int argc, char *argv[])
{
    AllocConsole();  // Creates a console window
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);  // Redirects cout to console

    // Your code here
    Map2D map;
    map.drawMapASCII();

    system("pause");  // Keeps console open to see the output
    QApplication app(argc, argv);
    PROJETVOLANGTBL window;
    window.show();
    return app.exec();
}

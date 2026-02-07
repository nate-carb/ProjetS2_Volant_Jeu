#include "PROJETVOLANGTBL.h"
#include "map2d.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <windows.h>
#include <iostream>
#include <QDebug>
#include <chrono>
/*
int main(int argc, char *argv[])
{
    // Better console allocation
    if (AllocConsole()) {
        FILE* fp = nullptr;
        if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0) {
            // freopen failed, but continue anyway
        }
        if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0) {
            // stderr redirect failed
        }
        // Optional: also redirect stdin for input
        if (freopen_s(&fp, "CONIN$", "r", stdin) != 0) {
            // stdin redirect failed
        }
    }
   
    // Your code here
    Map2D map;
	map.trackGenerator();
	
    
    system("pause");  // Keeps console open to see the output
    QApplication app(argc, argv);
    PROJETVOLANGTBL window;
    window.show();
    return app.exec();
	
}
*/

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
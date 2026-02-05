#include "PROJETVOLANGTBL.h"
#include "map2d.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    Map2D track;
    track.drawMapASCII();
    
    QApplication app(argc, argv);
    PROJETVOLANGTBL window;
    window.show();
    return app.exec();
}

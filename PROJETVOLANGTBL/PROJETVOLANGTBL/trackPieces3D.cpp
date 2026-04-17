#include "trackPieces3D.h"

QMap<int, TrackPiece3DInfo> TrackPiece3DLibrary::pieceLibrary;

void TrackPiece3DLibrary::loadAllModels()
{

    TrackPiece3DInfo straight;
    straight.pieceId = STRAIGHT;
    straight.modelPath = "models/straight.obj";  
    straight.size = QVector3D(40, 20, 2.5); // Dimensions de la piece 3D
    straight.entryAngle = 0; 
    straight.exitAngle = 0;
    straight.exitOffset = QVector3D(0, 0, 40);  
    pieceLibrary[STRAIGHT] = straight;

    TrackPiece3DInfo left45;
    left45.pieceId = VIRAGE_45LEFT;
    left45.modelPath = "models/curve_45_left.obj";
    left45.size = QVector3D(30, 30, 2);
    left45.entryAngle = 0;
	left45.exitAngle = 45;
    left45.exitOffset = QVector3D(5 * sin(qDegreesToRadians(45)),0,15*sin(qDegreesToRadians(45)));  
    pieceLibrary[VIRAGE_45LEFT] = left45;

    TrackPiece3DInfo right45;
    right45.pieceId = VIRAGE_45RIGHT;
    right45.modelPath = "models/curve_45_right.obj";
    right45.size = QVector3D(70, 1, 40);
    right45.entryAngle = 0;
    right45.exitAngle = -45;
    right45.exitOffset = QVector3D(35, 0, 35);
    pieceLibrary[VIRAGE_45RIGHT] = right45;

    TrackPiece3DInfo left90;
    left90.pieceId = VIRAGE_90LEFT;
    left90.modelPath = "models/curve_90_left.obj";
    left90.size = QVector3D(70, 1, 70);
    left90.entryAngle = 0;
    left90.exitAngle = 90;
    left90.exitOffset = QVector3D(-50, 0, 0);  
    pieceLibrary[VIRAGE_90LEFT] = left90;

    TrackPiece3DInfo right90;
    right90.pieceId = VIRAGE_90RIGHT;
    right90.modelPath = "models/curve_90_right.obj";
    right90.size = QVector3D(70, 1, 70);
    right90.entryAngle = 0;
    right90.exitAngle = -90;
    right90.exitOffset = QVector3D(50, 0, 0);  
    pieceLibrary[VIRAGE_90RIGHT] = right90;
}

TrackPiece3DInfo TrackPiece3DLibrary::getPieceInfo(int pieceId)
{
    if (pieceLibrary.contains(pieceId)) {
        return pieceLibrary[pieceId];
    }

    return pieceLibrary[STRAIGHT];
}
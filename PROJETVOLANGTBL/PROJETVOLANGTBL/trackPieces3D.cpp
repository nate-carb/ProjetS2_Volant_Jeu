#include "trackPieces3D.h"

QMap<int, TrackPiece3DInfo> TrackPiece3DLibrary::pieceLibrary;

void TrackPiece3DLibrary::loadAllModels()
{
    // STRAIGHT piece
    TrackPiece3DInfo straight;
    straight.pieceId = STRAIGHT;
    straight.modelPath = "models/straight.obj";  // Relative to exe
    straight.size = QVector3D(100, 1, 40);
    straight.entryAngle = 0;
    straight.exitAngle = 0;
    straight.exitOffset = QVector3D(0, 0, 100);  // Move 100 forward (Z-axis)
    pieceLibrary[STRAIGHT] = straight;

    // VIRAGE_45LEFT piece
    TrackPiece3DInfo left45;
    left45.pieceId = VIRAGE_45LEFT;
    left45.modelPath = "models/curve_45_left.obj";
    left45.size = QVector3D(70, 1, 40);
    left45.entryAngle = 0;
    left45.exitAngle = 45;
    left45.exitOffset = QVector3D(-35, 0, 35);  // Diagonal for 45° turn
    pieceLibrary[VIRAGE_45LEFT] = left45;

    // VIRAGE_45RIGHT piece
    TrackPiece3DInfo right45;
    right45.pieceId = VIRAGE_45RIGHT;
    right45.modelPath = "models/curve_45_right.obj";
    right45.size = QVector3D(70, 1, 40);
    right45.entryAngle = 0;
    right45.exitAngle = -45;
    right45.exitOffset = QVector3D(35, 0, 35);
    pieceLibrary[VIRAGE_45RIGHT] = right45;

    // VIRAGE_90LEFT piece
    TrackPiece3DInfo left90;
    left90.pieceId = VIRAGE_90LEFT;
    left90.modelPath = "models/curve_90_left.obj";
    left90.size = QVector3D(70, 1, 70);
    left90.entryAngle = 0;
    left90.exitAngle = 90;
    left90.exitOffset = QVector3D(-50, 0, 0);  // 90° left turn
    pieceLibrary[VIRAGE_90LEFT] = left90;

    // VIRAGE_90RIGHT piece
    TrackPiece3DInfo right90;
    right90.pieceId = VIRAGE_90RIGHT;
    right90.modelPath = "models/curve_90_right.obj";
    right90.size = QVector3D(70, 1, 70);
    right90.entryAngle = 0;
    right90.exitAngle = -90;
    right90.exitOffset = QVector3D(50, 0, 0);  // 90° right turn
    pieceLibrary[VIRAGE_90RIGHT] = right90;
}

TrackPiece3DInfo TrackPiece3DLibrary::getPieceInfo(int pieceId)
{
    if (pieceLibrary.contains(pieceId)) {
        return pieceLibrary[pieceId];
    }

    // Return default straight if not found
    return pieceLibrary[STRAIGHT];
}
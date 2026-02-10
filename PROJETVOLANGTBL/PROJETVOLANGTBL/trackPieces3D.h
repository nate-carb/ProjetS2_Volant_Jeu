// TrackPiece3D.h
#pragma once
#include <QString>
#include <QVector3D>
#include <QQuaternion>

#include "Track.h"

struct TrackPiece3DInfo {
    int pieceId;                  // Same as 2D (STRAIGHT, VIRAGE_45LEFT, etc.)
    QString modelPath;            // Path to 3D model file (.obj, .fbx)
    QVector3D size;               // Physical dimensions (length, width, height)
    float entryAngle;             // Angle at start of piece
    float exitAngle;              // Angle at end of piece
    QVector3D exitOffset;         // Where next piece connects
};

class TrackPiece3DLibrary {
public:
    static TrackPiece3DInfo getPieceInfo(int pieceId);
    static void loadAllModels();

private:
    static QMap<int, TrackPiece3DInfo> pieceLibrary;
};
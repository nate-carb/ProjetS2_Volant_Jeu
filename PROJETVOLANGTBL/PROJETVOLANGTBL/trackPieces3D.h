#pragma once
#include <QString>
#include <QVector3D>
#include <QQuaternion>
#include "Track.h"

struct TrackPiece3DInfo {
    int pieceId;                  
    QString modelPath;            
    QVector3D size;               
    float entryAngle;             
    float exitAngle;              
    QVector3D exitOffset;         
};

class TrackPiece3DLibrary {
public:
    static TrackPiece3DInfo getPieceInfo(int pieceId);
    static void loadAllModels();

private:
    static QMap<int, TrackPiece3DInfo> pieceLibrary;
};
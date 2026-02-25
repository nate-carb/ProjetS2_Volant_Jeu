// Track3DBuilder.h
#pragma once
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <vector>
#include "Track.h"
#include "TrackPieces3D.h"

struct PlacedPiece3D {
    Qt3DCore::QEntity* entity;
    Qt3DCore::QTransform* transform;
    int pieceId;
    QVector3D position;
    QQuaternion rotation;
};

class Track3DBuilder {
public:
    Track3DBuilder(Qt3DCore::QEntity* rootEntity);

    bool loadFromTrkFile(const QString& filename);
    void clear();

    std::vector<PlacedPiece3D> getPlacedPieces() const { return placedPieces; }

private:
    Qt3DCore::QEntity* rootEntity;
    std::vector<PlacedPiece3D> placedPieces;

    QVector3D currentPosition;
    float currentAngle;

    void placePiece(int pieceId);
    Qt3DCore::QEntity* createPieceEntity(const TrackPiece3DInfo& info);
};

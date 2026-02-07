// Track3DBuilder.cpp
#include "TrackBuilder3D.h"
#include <Qt3DRender/QMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <QUrl>

Track3DBuilder::Track3DBuilder(Qt3DCore::QEntity* root)
    : rootEntity(root)
    , currentPosition(0, 0, 0)
    , currentAngle(0)
{
    TrackPiece3DLibrary::loadAllModels();
}

bool Track3DBuilder::loadFromTrkFile(const QString& filename)
{
    clear();

    // Load the 2D track
    Track track2D;
    if (!track2D.loadFromFile(filename.toStdString())) {
        qWarning() << "Failed to load track from" << filename;
        return false;
    }

    // Get the piece list
    std::vector<int> pieces = track2D.getPiecesList();

    qDebug() << "Building 3D track with" << pieces.size() << "pieces";

    // Reset position and angle
    currentPosition = QVector3D(0, 0, 0);
    currentAngle = 0;

    // Place each piece
    for (int pieceId : pieces) {
        placePiece(pieceId);
    }

    qDebug() << "3D track built successfully";
    return true;
}

void Track3DBuilder::placePiece(int pieceId)
{
    TrackPiece3DInfo info = TrackPiece3DLibrary::getPieceInfo(pieceId);

    // Create 3D entity for this piece
    Qt3DCore::QEntity* pieceEntity = createPieceEntity(info);

    // Create transform
    Qt3DCore::QTransform* transform = new Qt3DCore::QTransform();

    // Set position
    transform->setTranslation(currentPosition);

    // Set rotation (rotate around Y-axis)
    QQuaternion rotation = QQuaternion::fromAxisAndAngle(
        QVector3D(0, 1, 0),  // Y-axis (up)
        currentAngle
    );
    transform->setRotation(rotation);

    pieceEntity->addComponent(transform);

    // Store placed piece info
    PlacedPiece3D placed;
    placed.entity = pieceEntity;
    placed.transform = transform;
    placed.pieceId = pieceId;
    placed.position = currentPosition;
    placed.rotation = rotation;
    placedPieces.push_back(placed);

    // Calculate next position and angle
    // Rotate exitOffset by current angle
    QMatrix4x4 rotMatrix;
    rotMatrix.rotate(currentAngle, 0, 1, 0);
    QVector3D rotatedOffset = rotMatrix.map(info.exitOffset);
    //QVector3D rotatedOffset = rotMatrix * info.exitOffset;

    currentPosition += rotatedOffset;
    currentAngle += info.exitAngle;
	if (currentAngle >= 360) currentAngle -= 360;

    qDebug() << "Placed piece" << pieceId
        << "at" << placed.position
        << "angle" << currentAngle;
}

Qt3DCore::QEntity* Track3DBuilder::createPieceEntity(const TrackPiece3DInfo& info)
{
    Qt3DCore::QEntity* entity = new Qt3DCore::QEntity(rootEntity);

    // Load 3D mesh
    Qt3DRender::QMesh* mesh = new Qt3DRender::QMesh();
    mesh->setSource(QUrl::fromLocalFile(info.modelPath));
    entity->addComponent(mesh);

    // Add material (simple gray for now)
    Qt3DExtras::QPhongMaterial* material = new Qt3DExtras::QPhongMaterial();
    material->setDiffuse(QColor(100, 100, 100));
    material->setAmbient(QColor(50, 50, 50));
    material->setSpecular(QColor(200, 200, 200));
    material->setShininess(50.0f);
    entity->addComponent(material);

    return entity;
}

void Track3DBuilder::clear()
{
    // Delete all placed pieces
    for (auto& piece : placedPieces) {
        if (piece.entity) {
            delete piece.entity;
        }
    }
    placedPieces.clear();

    currentPosition = QVector3D(0, 0, 0);
    currentAngle = 0;
}
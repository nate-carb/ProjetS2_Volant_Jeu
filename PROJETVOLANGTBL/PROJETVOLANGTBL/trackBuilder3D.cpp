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

    Track track2D;
    if (!track2D.loadFromFile(filename.toStdString())) {
        qWarning() << "Failed to load track from" << filename;
        return false;
    }

    std::vector<int> pieces = track2D.getPiecesList();

    qDebug() << "Building 3D track with" << pieces.size() << "pieces";

    currentPosition = QVector3D(0, 0, 0);
    currentAngle = 0;

    for (int pieceId : pieces) {
        placePiece(pieceId);
    }

    qDebug() << "3D track built successfully";
    return true;
}

void Track3DBuilder::placePiece(int pieceId)
{
    TrackPiece3DInfo info = TrackPiece3DLibrary::getPieceInfo(pieceId);

    Qt3DCore::QEntity* pieceEntity = createPieceEntity(info);

    Qt3DCore::QTransform* transform = new Qt3DCore::QTransform();

    transform->setTranslation(currentPosition);

    QQuaternion rotation = QQuaternion::fromAxisAndAngle(
        QVector3D(0, 1, 0),  
        currentAngle
    );
    transform->setRotation(rotation);

    pieceEntity->addComponent(transform);

    PlacedPiece3D placed;
    placed.entity = pieceEntity;
    placed.transform = transform;
    placed.pieceId = pieceId;
    placed.position = currentPosition;
    placed.rotation = rotation;
    placedPieces.push_back(placed);

    QMatrix4x4 rotMatrix;
    rotMatrix.rotate(currentAngle, 0, 1, 0);
    QVector3D rotatedOffset = rotMatrix.map(info.exitOffset);

    currentPosition += rotatedOffset;
    currentAngle += info.exitAngle;

    qDebug() << "Placed piece" << pieceId
        << "at" << placed.position
        << "angle" << currentAngle;
}

Qt3DCore::QEntity* Track3DBuilder::createPieceEntity(const TrackPiece3DInfo& info)
{
    Qt3DCore::QEntity* entity = new Qt3DCore::QEntity(rootEntity);

    Qt3DRender::QMesh* mesh = new Qt3DRender::QMesh();
    mesh->setSource(QUrl::fromLocalFile(info.modelPath));
    entity->addComponent(mesh);

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
    for (auto& piece : placedPieces) {
        if (piece.entity) {
            delete piece.entity;
        }
    }
    placedPieces.clear();

    currentPosition = QVector3D(0, 0, 0);
    currentAngle = 0;
}
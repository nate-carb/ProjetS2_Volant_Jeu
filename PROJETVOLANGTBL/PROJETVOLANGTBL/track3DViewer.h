#pragma once

#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QFirstPersonCameraController>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <QVector3D>
#include <QVector>
#include <Qt3DRender/QMesh>

#include "Track.h"
#include "Vehicule.h"

class Track3DViewer : public Qt3DExtras::Qt3DWindow
{
    Q_OBJECT

public:
    explicit Track3DViewer(QScreen* screen = nullptr);
    ~Track3DViewer();

    // Call this to load and display a track
    void setTrack(Track* track);

    // Call this every game frame to update car position
    void updateVehicule(Vehicule* vehicule);

    // Switch between first-person and orbit (debug) camera
    void setFirstPersonMode(bool enabled);

public slots:
    void onUpdateFrame();

private:
    // Scene root
    Qt3DCore::QEntity* m_rootEntity = nullptr;

    // Camera
    Qt3DRender::QCamera* m_camera = nullptr;
    Qt3DExtras::QOrbitCameraController* m_orbitController = nullptr;
    Qt3DExtras::QFirstPersonCameraController* m_fpController = nullptr;
    bool m_firstPersonMode = true;

    // Track mesh entity
    Qt3DCore::QEntity* m_trackEntity = nullptr;

    // Car entity (simple box for now)
    Qt3DCore::QEntity* m_carEntity = nullptr;
    Qt3DCore::QTransform* m_carTransform = nullptr;

    // Grass/ground entity
    Qt3DCore::QEntity* m_groundEntity = nullptr;

    // Internal helpers
    void buildScene();
    void buildTrackMesh(Track* track);
    void buildCar();
    void buildGround();

    Qt3DCore::QEntity* createBox(Qt3DCore::QEntity* parent,
        QVector3D size,
        QVector3D position,
        QColor color);

    // Stored track pointer (not owned)
    Track* m_track = nullptr;
    Vehicule* m_vehicule = nullptr;
};
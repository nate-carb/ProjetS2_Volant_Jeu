#pragma once

#include <QKeyEvent>
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
#include <Qt3DExtras/QSkyboxEntity>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QGeometryRenderer>
#include <QMap>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QDirectionalLight>
#include "Track.h"
#include "Vehicule.h"
#include "HUDOverlay.h"
#include "DaeLoader.h"
#include "MeshInstance.h"

class Track3DViewer : public Qt3DExtras::Qt3DWindow
{
    Q_OBJECT

public:
    explicit Track3DViewer(QScreen* screen = nullptr);
    ~Track3DViewer();

    void setTrack(Track* track);

    void updateVehicule(Vehicule* vehicule);

    void setFirstPersonMode(bool enabled);

	void changeCameraMode(); 
    bool eventFilter(QObject* obj, QEvent* event) override;


    
public slots:
    void onUpdateFrame();

private:

    Qt3DCore::QEntity* m_rootEntity = nullptr;

    Qt3DCore::QEntity* m_sceneRoot = nullptr;

    Qt3DRender::QCamera* m_camera = nullptr;
    Qt3DExtras::QOrbitCameraController* m_orbitController = nullptr;
    Qt3DExtras::QFirstPersonCameraController* m_fpController = nullptr;

    float m_cameraYaw = 15.0f; 
    float m_cameraLag = 0.35f; 
    bool m_firstPersonMode = true;

	bool carCamthird = true; 
    

	// Skybox
    Qt3DExtras::QSkyboxEntity* m_skybox = nullptr;
    Qt3DCore::QTransform* m_skyTransform = nullptr;
    
    // Track 
    Qt3DCore::QEntity* m_trackEntity = nullptr;

    // Char 
    Qt3DCore::QEntity* m_carEntity = nullptr;
    Qt3DCore::QTransform* m_carTransform = nullptr;

    // Grass/ground 
    Qt3DCore::QEntity* m_groundEntity = nullptr;

	// Decor 
    QVector<Qt3DCore::QEntity*> m_decorEntities;
    
    QMap<QString, Qt3DRender::QSceneLoader*> m_loaderCache; 

	// Checkpoints
    QVector<Qt3DCore::QEntity*> m_checkpointEntities;

	// Bezier 
    void buildBezierWalls(Track* track);
    QVector<Qt3DCore::QEntity*> m_wallEntities;

    // Helpers
    void buildScene(Track* track);
    void buildSkybox(Track* track);
    void buildTrackMesh(Track* track);
    void buildCar();
    void buildDecors(Track* track);
    void buildGround(Track* track);
    void buildCheckpoints(Track* track);
    void buildLights();      
    bool m_lightsBuilt = false;

    // Decors 
    QVector<Qt3DCore::QEntity*> m_instancedDecorEntities;
    void buildInstancedDecors(Track* track);

    Qt3DCore::QEntity* createBox(Qt3DCore::QEntity* parent,
        QVector3D size,
        QVector3D position,
        QColor color);

    Track* m_track = nullptr;
    Vehicule* m_vehicule = nullptr;

    //HUD Overlay en 3D
    HUDOverlay* m_hud = nullptr;
signals:
    void trackUpdated(Track* track);

protected:
    void keyPressEvent(QKeyEvent* event) override;
};
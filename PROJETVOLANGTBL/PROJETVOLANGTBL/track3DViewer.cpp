// Track3DViewer.cpp
#include "Track3DViewer.h"
#include <QVBoxLayout>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QPointLight>
#include <Qt3DCore/QTransform>

Track3DViewer::Track3DViewer(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Create 3D view
    view3D = new Qt3DExtras::Qt3DWindow();
    QWidget* container = QWidget::createWindowContainer(view3D, this);
    layout->addWidget(container);

    // Create root entity
    rootEntity = new Qt3DCore::QEntity();
    sceneRoot = new Qt3DCore::QEntity(rootEntity);

    // Setup camera
    setupCamera();

    // Setup lighting
    setupLighting();

    // Create track builder
    trackBuilder = new Track3DBuilder(sceneRoot);

    // Set the root entity
    view3D->setRootEntity(rootEntity);
}

void Track3DViewer::setupCamera()
{
    Qt3DRender::QCamera* camera = view3D->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 10000.0f);
    camera->setPosition(QVector3D(0, 200, 400));
    camera->setViewCenter(QVector3D(0, 0, 0));

    // Add camera controller
    Qt3DExtras::QOrbitCameraController* camController =
        new Qt3DExtras::QOrbitCameraController(rootEntity);
    camController->setLinearSpeed(50.0f);
    camController->setLookSpeed(180.0f);
    camController->setCamera(camera);
}

void Track3DViewer::setupLighting()
{
    // Add a light
    Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight* light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(1.5f);
    lightEntity->addComponent(light);

    Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(QVector3D(0, 500, 0));
    lightEntity->addComponent(lightTransform);
}

void Track3DViewer::loadTrackFile(const QString& filename)
{
    trackBuilder->loadFromTrkFile(filename);
}

void Track3DViewer::clearTrack()
{
    trackBuilder->clear();
}
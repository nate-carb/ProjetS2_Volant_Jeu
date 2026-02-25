// Track3DViewer.h
#pragma once
#include <QWidget>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DExtras/QOrbitCameraController>
#include "TrackBuilder3D.h"

class Track3DViewer : public QWidget {
    Q_OBJECT

public:
    Track3DViewer(QWidget* parent = nullptr);

    void loadTrackFile(const QString& filename);
    void clearTrack();

private:
    Qt3DExtras::Qt3DWindow* view3D;
    Qt3DCore::QEntity* rootEntity;
    Qt3DCore::QEntity* sceneRoot;
    Track3DBuilder* trackBuilder;

    void setupCamera();
    void setupLighting();
};
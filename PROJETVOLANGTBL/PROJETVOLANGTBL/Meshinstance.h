#pragma once
#include "MeshConverter.h"
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QAttribute>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QGraphicsApiFilter>
#include <QMatrix4x4>
#include <QVector>

class MeshInstance
{
public:
    // Build instanced entities from a single MeshData
    // Returns one entity — one draw call
    static Qt3DCore::QEntity* build(
        const MeshData& mesh,
        const QVector<QMatrix4x4>& transforms,
        Qt3DCore::QEntity* parent);

    // Build instanced entities from a MeshDataList
    // Returns a parent entity containing one child per submesh
    // Each submesh = one draw call with correct material color
    static Qt3DCore::QEntity* buildFromList(
        const MeshDataList& meshList,
        const QVector<QMatrix4x4>& transforms,
        Qt3DCore::QEntity* parent);
};
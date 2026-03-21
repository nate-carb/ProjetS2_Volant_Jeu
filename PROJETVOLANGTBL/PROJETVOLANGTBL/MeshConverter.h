#pragma once
#include <QString>
#include <QVector>
#include <QVector3D>
#include <QVector2D>
#include <QColor>

// One vertex with position, normal and UV
struct MeshVertex {
    QVector3D position;
    QVector3D normal;
    QVector2D uv;
};

// One submesh — geometry + one material
struct MeshData {
    QVector<MeshVertex> vertices;
    QVector<quint32>    indices;
    QColor diffuse = QColor(200, 200, 200);
    QColor ambient = QColor(100, 100, 100);
    QColor specular = QColor(0, 0, 0);
    float  shininess = 0.0f;
    bool   valid = false;
};

// All submeshes from one .dae file (one per material)
struct MeshDataList {
    QVector<MeshData> meshes;
    bool valid = false;
};
#include "MeshInstance.h"
#include <QDir>
#include <QDebug>
#include <QUrl>

// ─────────────────────────────────────────────────────────────
// Internal helper — builds one instanced draw call for one
// MeshData (one material, N instances)
// ─────────────────────────────────────────────────────────────
static Qt3DCore::QEntity* buildSingleMesh(
    const MeshData& mesh,
    const QVector<QMatrix4x4>& transforms,
    Qt3DCore::QEntity* parent)
{
    if (!mesh.valid || transforms.isEmpty()) return nullptr;

    Qt3DCore::QEntity* entity = new Qt3DCore::QEntity(parent);

    // ── Vertex buffer ─────────────────────────────────────────
    QByteArray vertData;
    vertData.resize(mesh.vertices.size() * 8 * sizeof(float));
    float* vptr = reinterpret_cast<float*>(vertData.data());
    for (const MeshVertex& v : mesh.vertices) {
        *vptr++ = v.position.x();
        *vptr++ = v.position.y();
        *vptr++ = v.position.z();
        *vptr++ = v.normal.x();
        *vptr++ = v.normal.y();
        *vptr++ = v.normal.z();
        *vptr++ = v.uv.x();
        *vptr++ = v.uv.y();
    }
    Qt3DCore::QBuffer* vertexBuffer = new Qt3DCore::QBuffer(entity);
    vertexBuffer->setData(vertData);

    // ── Index buffer ──────────────────────────────────────────
    QByteArray idxData;
    idxData.resize(mesh.indices.size() * sizeof(quint32));
    memcpy(idxData.data(), mesh.indices.constData(),
        static_cast<size_t>(idxData.size()));
    Qt3DCore::QBuffer* indexBuffer = new Qt3DCore::QBuffer(entity);
    indexBuffer->setData(idxData);

    // ── Instance buffer — one mat4 per instance ───────────────
    QByteArray instData;
    instData.resize(transforms.size() * 16 * sizeof(float));
    float* iptr = reinterpret_cast<float*>(instData.data());
    for (const QMatrix4x4& m : transforms) {
        const float* d = m.constData();
        for (int i = 0; i < 16; i++) *iptr++ = d[i];
    }
    Qt3DCore::QBuffer* instanceBuffer =
        new Qt3DCore::QBuffer(entity);
    instanceBuffer->setData(instData);

    // ── Geometry ──────────────────────────────────────────────
    Qt3DCore::QGeometry* geom = new Qt3DCore::QGeometry(entity);

    auto addAttr = [&](const QString& name,
        int byteOffset, int vertexSize,
        Qt3DCore::QBuffer* buf,
        int byteStride, uint count,
        int divisor = 0)
        {
            Qt3DCore::QAttribute* attr =
                new Qt3DCore::QAttribute(geom);
            attr->setName(name);
            attr->setVertexBaseType(Qt3DCore::QAttribute::Float);
            attr->setVertexSize(vertexSize);
            attr->setAttributeType(
                Qt3DCore::QAttribute::VertexAttribute);
            attr->setBuffer(buf);
            attr->setByteOffset(static_cast<uint>(byteOffset));
            attr->setByteStride(static_cast<uint>(byteStride));
            attr->setCount(count);
            attr->setDivisor(static_cast<uint>(divisor));
            geom->addAttribute(attr);
        };

    const int  vStride = 8 * sizeof(float);
    const uint vertCount = static_cast<uint>(mesh.vertices.size());
    const uint instCount = static_cast<uint>(transforms.size());
    const int  iStride = 16 * sizeof(float);

    addAttr(Qt3DCore::QAttribute::defaultPositionAttributeName(),
        0, 3, vertexBuffer, vStride, vertCount);
    addAttr(Qt3DCore::QAttribute::defaultNormalAttributeName(),
        3 * sizeof(float), 3, vertexBuffer, vStride, vertCount);
    addAttr(
        Qt3DCore::QAttribute::
        defaultTextureCoordinateAttributeName(),
        6 * sizeof(float), 2, vertexBuffer, vStride, vertCount);

    // Instance matrix: 4 columns of vec4, divisor=1
    for (int col = 0; col < 4; col++)
        addAttr(QString("instanceModelMatrix%1").arg(col),
            col * 4 * static_cast<int>(sizeof(float)),
            4, instanceBuffer, iStride, instCount, 1);

    // Index attribute
    Qt3DCore::QAttribute* idxAttr =
        new Qt3DCore::QAttribute(geom);
    idxAttr->setAttributeType(
        Qt3DCore::QAttribute::IndexAttribute);
    idxAttr->setVertexBaseType(
        Qt3DCore::QAttribute::UnsignedInt);
    idxAttr->setBuffer(indexBuffer);
    idxAttr->setCount(static_cast<uint>(mesh.indices.size()));
    geom->addAttribute(idxAttr);

    // ── Renderer ──────────────────────────────────────────────
    Qt3DRender::QGeometryRenderer* renderer =
        new Qt3DRender::QGeometryRenderer(entity);
    renderer->setGeometry(geom);
    renderer->setPrimitiveType(
        Qt3DRender::QGeometryRenderer::Triangles);
    renderer->setInstanceCount(
        static_cast<int>(transforms.size()));

    // ── Custom instanced shader ───────────────────────────────
    Qt3DRender::QShaderProgram* shader =
        new Qt3DRender::QShaderProgram(entity);
    shader->setVertexShaderCode(
        Qt3DRender::QShaderProgram::loadSource(
            QUrl::fromLocalFile(
                QDir::currentPath() +
                "/shaders/instanced.vert")));
    shader->setFragmentShaderCode(
        Qt3DRender::QShaderProgram::loadSource(
            QUrl::fromLocalFile(
                QDir::currentPath() +
                "/shaders/instanced.frag")));

    QObject::connect(shader,
        &Qt3DRender::QShaderProgram::statusChanged,
        [shader](Qt3DRender::QShaderProgram::Status status) {
            if (status == Qt3DRender::QShaderProgram::Error)
                qDebug() << "Shader error:" << shader->log();
        });

    Qt3DRender::QRenderPass* renderPass =
        new Qt3DRender::QRenderPass(entity);
    renderPass->setShaderProgram(shader);

    Qt3DRender::QTechnique* technique =
        new Qt3DRender::QTechnique(entity);
    technique->graphicsApiFilter()->setApi(
        Qt3DRender::QGraphicsApiFilter::OpenGL);
    technique->graphicsApiFilter()->setProfile(
        Qt3DRender::QGraphicsApiFilter::CoreProfile);
    technique->graphicsApiFilter()->setMajorVersion(3);
    technique->graphicsApiFilter()->setMinorVersion(3);
    technique->addRenderPass(renderPass);

    Qt3DRender::QFilterKey* filterKey =
        new Qt3DRender::QFilterKey(entity);
    filterKey->setName("renderingStyle");
    filterKey->setValue("forward");
    technique->addFilterKey(filterKey);

    Qt3DRender::QEffect* effect =
        new Qt3DRender::QEffect(entity);
    effect->addTechnique(technique);

    Qt3DRender::QMaterial* mat =
        new Qt3DRender::QMaterial(entity);
    mat->setEffect(effect);

    // Colors directly from the parsed .dae material
    mat->addParameter(new Qt3DRender::QParameter(
        "ka", QVector4D(mesh.ambient.redF(),
            mesh.ambient.greenF(),
            mesh.ambient.blueF(), 1.0f)));
    mat->addParameter(new Qt3DRender::QParameter(
        "kd", QVector4D(mesh.diffuse.redF(),
            mesh.diffuse.greenF(),
            mesh.diffuse.blueF(), 1.0f)));
    mat->addParameter(new Qt3DRender::QParameter(
        "ks", QVector4D(mesh.specular.redF(),
            mesh.specular.greenF(),
            mesh.specular.blueF(), 1.0f)));
    mat->addParameter(new Qt3DRender::QParameter(
        "shininess",
        mesh.shininess > 0.0f ? mesh.shininess : 0.0f));

    entity->addComponent(renderer);
    entity->addComponent(mat);

    return entity;
}

// ─────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────

Qt3DCore::QEntity* MeshInstance::build(
    const MeshData& mesh,
    const QVector<QMatrix4x4>& transforms,
    Qt3DCore::QEntity* parent)
{
    return buildSingleMesh(mesh, transforms, parent);
}

Qt3DCore::QEntity* MeshInstance::buildFromList(
    const MeshDataList& meshList,
    const QVector<QMatrix4x4>& transforms,
    Qt3DCore::QEntity* parent)
{
    if (!meshList.valid || transforms.isEmpty()) return nullptr;

    // Parent entity groups all submesh entities together
    Qt3DCore::QEntity* groupEntity =
        new Qt3DCore::QEntity(parent);

    int submeshCount = 0;
    for (const MeshData& mesh : meshList.meshes) {
        if (!mesh.valid) continue;
        Qt3DCore::QEntity* sub =
            buildSingleMesh(mesh, transforms, groupEntity);
        if (sub) submeshCount++;
    }

    qDebug() << "MeshInstance::buildFromList -"
        << transforms.size() << "instances x"
        << submeshCount << "submeshes ="
        << (transforms.size() * submeshCount)
        << "draw calls";

    return groupEntity;
}
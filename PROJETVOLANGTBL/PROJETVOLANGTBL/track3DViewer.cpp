#include "Track3DViewer.h"
#include <qdir.h>

#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QSceneLoader>
#include <QUrl>

#include <QColor>
#include <QtMath>
#include <QTimer>

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
Track3DViewer::Track3DViewer(QScreen* screen)
    : Qt3DExtras::Qt3DWindow(screen)
{
    // Sky colour
    defaultFrameGraph()->setClearColor(QColor(135, 206, 235)); // light blue sky

    // Root entity – everything lives under here
    m_rootEntity = new Qt3DCore::QEntity();
    setRootEntity(m_rootEntity);

    buildScene();
}

Track3DViewer::~Track3DViewer() {}

// ─────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────
void Track3DViewer::setTrack(Track* track)
{
    m_track = track;

    // Remove old track entity if it exists
    if (m_trackEntity) {
        m_trackEntity->setParent(static_cast<Qt3DCore::QEntity*>(nullptr));
        delete m_trackEntity;
        m_trackEntity = nullptr;
    }

    buildTrackMesh(track);
    buildGround();
}
void Track3DViewer::updateVehicule(Vehicule* vehicule)
{
    m_vehicule = vehicule;
    if (!m_carTransform) return;

    float x = vehicule->getPosition().x();
    float y = vehicule->getPosition().y();
    float angle = vehicule->getAngle();

    // Car stays at game logic position
    m_carTransform->setTranslation(QVector3D(x, 0.0f, y)); // 0 = on the ground
    QQuaternion rot = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0),
        -qRadiansToDegrees(angle));
    m_carTransform->setRotation(rot);

    // Camera is calculated SEPARATELY and never touches m_carTransform
    if (m_firstPersonMode && m_camera) {
        float camOffsetBack = 20.0f;
        float camHeight = 15.0f;

        float camX = x - camOffsetBack * qCos(angle);
        float camZ = y - camOffsetBack * qSin(angle);

        float lookX = x + 30.0f * qCos(angle);
        float lookZ = y + 30.0f * qSin(angle);

        m_camera->setPosition(QVector3D(camX, camHeight, camZ));
        m_camera->setViewCenter(QVector3D(lookX, 5.0f, lookZ));
        m_camera->setUpVector(QVector3D(0, 1, 0));
    }
}
/*
void Track3DViewer::updateVehicule(Vehicule* vehicule)
{
    m_vehicule = vehicule;

    if (!m_carTransform) return;

    // Your 2D game uses (x, y) – we map to 3D as (x, 0, y)
    // (Qt3D Y is up, so your 2D Y becomes 3D Z)
    float x = vehicule->getPosition().x();
    float y = vehicule->getPosition().y();
    float angle = vehicule->getAngle(); // in radians

    // Update car transform
    m_carTransform->setTranslation(QVector3D(x, 5.0f, y)); // 5 = half car height

    // Convert 2D angle to 3D rotation around Y axis
    // Your angle 0 = pointing right (positive X), so we negate for Qt3D convention
    QQuaternion rot = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0),
        -qRadiansToDegrees(angle));
    m_carTransform->setRotation(rot);

    // ── First-person camera ──────────────────────────────────
    if (m_firstPersonMode && m_camera) {
        // Camera sits just behind and above the car
        float camOffsetBack = 20.0f;  // how far behind the car
        float camHeight = 15.0f; // eye height

        float camX = x - camOffsetBack * qCos(angle);
        float camZ = y - camOffsetBack * qSin(angle);

        // Look toward where the car is heading
        float lookX = x + 30.0f * qCos(angle);
        float lookZ = y + 30.0f * qSin(angle);

        m_camera->setPosition(QVector3D(camX, camHeight, camZ));
        m_camera->setViewCenter(QVector3D(lookX, 5.0f, lookZ));
        m_camera->setUpVector(QVector3D(0, 1, 0));
    }
}
*/
void Track3DViewer::setFirstPersonMode(bool enabled)
{
    m_firstPersonMode = enabled;

    if (m_orbitController)
        m_orbitController->setEnabled(!enabled);

    if (m_fpController)
        m_fpController->setEnabled(false); // we drive the camera manually from vehicule data
}

// ─────────────────────────────────────────────
// Scene setup
// ─────────────────────────────────────────────
void Track3DViewer::buildScene()
{
    // ── Camera ──────────────────────────────────────────────
    m_camera = camera();
	m_camera->lens()->setPerspectiveProjection(70.0f, 16.0f / 9.0f, 0.1f, 5000.0f); // fov, aspect, near, far
	m_camera->setPosition(QVector3D(0, 300, 300)); // initial position (overhead) 
	m_camera->setViewCenter(QVector3D(0, 0, 0)); // look at the origin initially
	m_camera->setUpVector(QVector3D(0, 1, 0)); // Y is up QVector3d(x, y, z)

    // ── Orbit controller (useful for debug / top-down view) ──
    m_orbitController = new Qt3DExtras::QOrbitCameraController(m_rootEntity);
    m_orbitController->setCamera(m_camera);
    m_orbitController->setLinearSpeed(300.0f);
    m_orbitController->setLookSpeed(180.0f);
    m_orbitController->setEnabled(!m_firstPersonMode);

    // ── Directional light (sun) ──────────────────────────────
    Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QDirectionalLight* light = new Qt3DRender::QDirectionalLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(1.0f);
    light->setWorldDirection(QVector3D(-0.5f, -1.0f, -0.3f));
    lightEntity->addComponent(light);

    // ── Car placeholder ──────────────────────────────────────
    buildCar();
}

// ─────────────────────────────────────────────
// Track mesh generation
// ─────────────────────────────────────────────
void Track3DViewer::buildTrackMesh(Track* track)
{
	// Get left and right edge points from your Track object
    const auto& left = track->getTrackEdges().left;
    const auto& right = track->getTrackEdges().right;

	// We need at least 2 points on each edge to form a quad segment
    if (left.size() < 2 || right.size() < 2) return;

	// Use the smaller of the two edge point counts to avoid out-of-bounds
    size_t n = qMin(left.size(), right.size());

    // ── Build vertex buffer ──────────────────────────────────
    // Each quad = 4 vertices (left[i], left[i+1], right[i], right[i+1])
    // Each quad = 2 triangles = 6 indices
    QVector<float> vertices;
    QVector<quint32> indices;

	// Reserve space to avoid reallocations
    vertices.reserve(static_cast<int>(n) * 2 * 3); // x,y,z per vertex
    indices.reserve(static_cast<int>((n - 1)) * 6);

    // Flatten left then right: left[0]…left[n-1], right[0]…right[n-1]
    // Your 2D (x,y) → 3D (x, 0, y)
    for (size_t i = 0; i < n; i++) {
        vertices << left[i].x() << 0.0f << left[i].y();
    }
    for (size_t i = 0; i < n; i++) {
        vertices << right[i].x() << 0.0f << right[i].y();
    }
    // Normal buffer – all pointing up since track is flat
    QVector<float> normals;
    normals.reserve(static_cast<int>(n) * 2 * 3);
    for (size_t i = 0; i < n * 2; i++) {
        normals << 0.0f << 1.0f << 0.0f;
    }

    // Indices: for each segment i, two triangles
    // left[i]=i,  left[i+1]=i+1
    // right[i]=n+i, right[i+1]=n+i+1
    for (quint32 i = 0; i < static_cast<quint32>(n - 1); i++) {
		quint32 l0 = i; // left[i]
		quint32 l1 = i + 1; // left[i+1]
		quint32 r0 = static_cast<quint32>(n) + i;   // right[i]
		quint32 r1 = static_cast<quint32>(n) + i + 1;  // right[i+1]


        // Triangle 1
        //indices << l0 << r0 << l1;
        //// Triangle 2
        //indices << l1 << r0 << r1;

		//reverse oriantation 
        //Triangle 1
        indices << l0 << l1 << r0;
        // Triangle 2
        indices << l1 << r1 << r0;
    }

    // ── Qt3D geometry objects ────────────────────────────────
    m_trackEntity = new Qt3DCore::QEntity(m_rootEntity);

	// Geometry renderer links the geometry to the entity and specifies how to render it
    Qt3DRender::QGeometryRenderer* renderer = new Qt3DRender::QGeometryRenderer(m_trackEntity);

	// Geometry holds vertex and index buffers + attributes
    //Qt3DCore::QGeometry* geometry = new Qt3DCore::QGeometry(m_trackEntity);
    Qt3DCore::QGeometry* geometry = new Qt3DCore::QGeometry(renderer);

    // Vertex buffer
    Qt3DCore::QBuffer* vertexBuffer = new Qt3DCore::QBuffer(geometry);
	// We can directly use the raw data from our QVector<float> as a QByteArray for the buffer
	QByteArray vertexData(reinterpret_cast<const char*>(vertices.constData()), //reinterpret as bytes
        vertices.size() * sizeof(float));
    vertexBuffer->setData(vertexData);

	// Position attribute 3 floats per vertex
	Qt3DCore::QAttribute* posAttr = new Qt3DCore::QAttribute(geometry); //QAttribute describes how to interpret the vertex buffer data
	// We use the default position attribute name so that Qt3D's built-in shaders can recognize it
    posAttr->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
	// We have 3 floats per vertex (x, y, z)
    posAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
    posAttr->setVertexSize(3);
    posAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
	// Link the attribute to our vertex buffer
    posAttr->setBuffer(vertexBuffer);
    posAttr->setByteStride(3 * sizeof(float)); 
	posAttr->setCount(static_cast<uint>(n * 2)); // total vertex count (left + right)
	geometry->addAttribute(posAttr); // add the attribute to the geometry

    Qt3DCore::QBuffer* normalBuffer = new Qt3DCore::QBuffer(geometry);
    normalBuffer->setData(QByteArray(reinterpret_cast<const char*>(normals.constData()),
        normals.size() * sizeof(float)));

    Qt3DCore::QAttribute* normAttr = new Qt3DCore::QAttribute(geometry);
    normAttr->setName(Qt3DCore::QAttribute::defaultNormalAttributeName());
    normAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
    normAttr->setVertexSize(3);
    normAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    normAttr->setBuffer(normalBuffer);
    normAttr->setByteStride(3 * sizeof(float));
    normAttr->setCount(static_cast<uint>(n * 2));
    geometry->addAttribute(normAttr);

	// Index buffer same as above, but with unsigned int indices
    Qt3DCore::QBuffer* indexBuffer = new Qt3DCore::QBuffer(geometry);
    QByteArray indexData(reinterpret_cast<const char*>(indices.constData()),
        indices.size() * sizeof(quint32));
    indexBuffer->setData(indexData);

    Qt3DCore::QAttribute* indexAttr = new Qt3DCore::QAttribute(geometry);
    indexAttr->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
    indexAttr->setVertexBaseType(Qt3DCore::QAttribute::UnsignedInt);
    indexAttr->setBuffer(indexBuffer);
    indexAttr->setCount(static_cast<uint>(indices.size()));
    geometry->addAttribute(indexAttr);

	// Geometry renderer links the geometry to the entity and specifies how to render it
    //Qt3DRender::QGeometryRenderer* renderer = new Qt3DRender::QGeometryRenderer(m_trackEntity);
    renderer->setGeometry(geometry);
	renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles); // the type of primitives we defined in our index buffer (triangles)

    // Material – grey tarmac colour
    Qt3DExtras::QPhongMaterial* material = new Qt3DExtras::QPhongMaterial(m_trackEntity);
    material->setDiffuse(QColor(60, 60, 60));    // dark grey = tarmac
	material->setAmbient(QColor(40, 40, 40)); // ambient is usually darker than diffuse
    material->setShininess(5.0f);

	m_trackEntity->addComponent(renderer); // add the geometry renderer to the entity
	m_trackEntity->addComponent(material); // add the material to the entity

    /*
    // TEMPORARY DEBUG – replace custom mesh with a visible box
    Qt3DCore::QEntity* debugBox = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DExtras::QCuboidMesh* debugMesh = new Qt3DExtras::QCuboidMesh();
    debugMesh->setXExtent(100.0f);
    debugMesh->setYExtent(10.0f);
    debugMesh->setZExtent(100.0f);
    Qt3DExtras::QPhongMaterial* debugMat = new Qt3DExtras::QPhongMaterial(debugBox);
    debugMat->setDiffuse(QColor(255, 0, 255)); // bright pink, hard to miss
    debugBox->addComponent(debugMesh);
    debugBox->addComponent(debugMat);
    */
    
    
    //--------------------------------------------------------------------
	//red and white kerbs along the edges, alternating every 10 units
    //--------------------------------------------------------------------
    auto buildKerb = [&](const std::vector<QVector2D>& edge, const std::vector<QVector2D>& center, bool flipWinding) {
        float kerbWidth = 5.0f;
        float kerbHeight = 0.2f;
        float segmentLength = 10.0f;
        float accumulated = 0.0f;
        bool isRed = true;

        // Two batches - one per color
        QVector<float>   redVerts, whiteVerts;
        QVector<quint32> redIdx, whiteIdx;
        QVector<float>   redNormals, whiteNormals;
        quint32 redCount = 0, whiteCount = 0;

        for (size_t i = 0; i < edge.size() - 1; i++) {
            QVector2D a = edge[i];
            QVector2D b = edge[i + 1];

            float segLen = (b - a).length();
            accumulated += segLen;
            if (accumulated >= segmentLength) {
                accumulated = 0.0f;
                isRed = !isRed;
            }

            QVector2D midEdge = (a + b) * 0.5f;
            QVector2D midCenter = (center[i] + center[i + 1]) * 0.5f;
            QVector2D toCenter = (midCenter - midEdge).normalized();
            QVector2D perp = toCenter * kerbWidth;

            QVector2D a_inner = a;
            QVector2D a_outer = a + perp;
            QVector2D b_inner = b;
            QVector2D b_outer = b + perp;

            auto& verts = isRed ? redVerts : whiteVerts;
            auto& idx = isRed ? redIdx : whiteIdx;
            auto& normals = isRed ? redNormals : whiteNormals;
            quint32& base = isRed ? redCount : whiteCount;

            verts << a_inner.x() << kerbHeight << a_inner.y()
                << a_outer.x() << kerbHeight << a_outer.y()
                << b_inner.x() << kerbHeight << b_inner.y()
                << b_outer.x() << kerbHeight << b_outer.y();

            for (int j = 0; j < 4; j++)
                normals << 0.0f << 1.0f << 0.0f;

            if (flipWinding) {
                idx << base + 0 << base + 2 << base + 1;
                idx << base + 1 << base + 2 << base + 3;
            }
            else {
                idx << base + 0 << base + 1 << base + 2;
                idx << base + 2 << base + 1 << base + 3;
            }
            base += 4;
        }

        // Helper to create one entity from batched geometry
        auto makeEntity = [&](QVector<float>& verts, QVector<float>& normals,
            QVector<quint32>& idx, QColor color) {
                if (verts.isEmpty()) return;

                Qt3DCore::QEntity* entity = new Qt3DCore::QEntity(m_rootEntity);
                Qt3DRender::QGeometryRenderer* renderer = new Qt3DRender::QGeometryRenderer(entity);
                Qt3DCore::QGeometry* geom = new Qt3DCore::QGeometry(renderer);

                Qt3DCore::QBuffer* vb = new Qt3DCore::QBuffer(geom);
                vb->setData(QByteArray(reinterpret_cast<const char*>(verts.constData()),
                    verts.size() * sizeof(float)));

                Qt3DCore::QAttribute* pos = new Qt3DCore::QAttribute(geom);
                pos->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
                pos->setVertexBaseType(Qt3DCore::QAttribute::Float);
                pos->setVertexSize(3);
                pos->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
                pos->setBuffer(vb);
                pos->setByteStride(3 * sizeof(float));
                pos->setCount(static_cast<uint>(verts.size() / 3));
                geom->addAttribute(pos);

                Qt3DCore::QBuffer* nb = new Qt3DCore::QBuffer(geom);
                nb->setData(QByteArray(reinterpret_cast<const char*>(normals.constData()),
                    normals.size() * sizeof(float)));

                Qt3DCore::QAttribute* norm = new Qt3DCore::QAttribute(geom);
                norm->setName(Qt3DCore::QAttribute::defaultNormalAttributeName());
                norm->setVertexBaseType(Qt3DCore::QAttribute::Float);
                norm->setVertexSize(3);
                norm->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
                norm->setBuffer(nb);
                norm->setByteStride(3 * sizeof(float));
                norm->setCount(static_cast<uint>(normals.size() / 3));
                geom->addAttribute(norm);

                Qt3DCore::QBuffer* ib = new Qt3DCore::QBuffer(geom);
                ib->setData(QByteArray(reinterpret_cast<const char*>(idx.constData()),
                    idx.size() * sizeof(quint32)));

                Qt3DCore::QAttribute* idxAttr = new Qt3DCore::QAttribute(geom);
                idxAttr->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
                idxAttr->setVertexBaseType(Qt3DCore::QAttribute::UnsignedInt);
                idxAttr->setBuffer(ib);
                idxAttr->setCount(static_cast<uint>(idx.size()));
                geom->addAttribute(idxAttr);

                renderer->setGeometry(geom);
                renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);

                Qt3DExtras::QPhongMaterial* mat = new Qt3DExtras::QPhongMaterial(entity);
                mat->setDiffuse(color);
                mat->setAmbient(color.darker(150));
                mat->setShininess(20.0f);

                entity->addComponent(renderer);
                entity->addComponent(mat);
            };

        // 2 entities per edge instead of hundreds
        makeEntity(redVerts, redNormals, redIdx, QColor(220, 30, 30));
        makeEntity(whiteVerts, whiteNormals, whiteIdx, Qt::white);
        };

    // Replace your old buildWall calls with:
		buildKerb(track->getTrackEdges().left, track->getCenterLine(), true); // left kerb with normal winding
		buildKerb(track->getTrackEdges().right, track->getCenterLine(), false); // right kerb with flipped winding to keep normals facing up
    // ───────────────────────────────────────── ─────────────────────────────────────────
    // ── Pit lane 3D mesh ─────────────────────────────────────────
    // ───────────────────────────────────────── ─────────────────────────────────────────
    if (track->hasPitLane()) {
        PitLane pit = track->getPitLane();

        // Helper lambda - same pattern as main track mesh
        auto buildPitMesh = [&](const std::vector<QVector2D>& left,
            const std::vector<QVector2D>& right,
            QColor color) {
                if (left.size() < 2 || right.size() < 2) {
                    qDebug() << "buildPitMesh skipped - left:" << left.size() << "right:" << right.size();
                    return;
                }
                if (left.size() != right.size()) {
                    qDebug() << "buildPitMesh size mismatch - left:" << left.size() << "right:" << right.size();
                    return;
                }
                if (left.size() < 2 || right.size() < 2) return;
                size_t pn = qMin(left.size(), right.size());

                QVector<float>   verts;
                QVector<float>   normals;
                QVector<quint32> idx;

                for (size_t i = 0; i < pn; i++)
                    verts << left[i].x() << 0.0f << left[i].y();
                for (size_t i = 0; i < pn; i++)
                    verts << right[i].x() << 0.0f << right[i].y();

                for (size_t i = 0; i < pn * 2; i++)
                    normals << 0.0f << 1.0f << 0.0f;

                for (quint32 i = 0; i < static_cast<quint32>(pn - 1); i++) {
                    quint32 l0 = i;
                    quint32 l1 = i + 1;
                    quint32 r0 = static_cast<quint32>(pn) + i;
                    quint32 r1 = static_cast<quint32>(pn) + i + 1;
                    idx << l0 << l1 << r0;
                    idx << l1 << r1 << r0;
                }

                Qt3DCore::QEntity* pitEntity = new Qt3DCore::QEntity(m_rootEntity);
                Qt3DRender::QGeometryRenderer* pitRenderer = new Qt3DRender::QGeometryRenderer(pitEntity);
                Qt3DCore::QGeometry* pitGeom = new Qt3DCore::QGeometry(pitRenderer);

                Qt3DCore::QBuffer* vb = new Qt3DCore::QBuffer(pitGeom);
                vb->setData(QByteArray(reinterpret_cast<const char*>(verts.constData()),
                    verts.size() * sizeof(float)));

                Qt3DCore::QAttribute* posAttr = new Qt3DCore::QAttribute(pitGeom);
                posAttr->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
                posAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
                posAttr->setVertexSize(3);
                posAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
                posAttr->setBuffer(vb);
                posAttr->setByteStride(3 * sizeof(float));
                posAttr->setCount(static_cast<uint>(pn * 2));
                pitGeom->addAttribute(posAttr);

                Qt3DCore::QBuffer* nb = new Qt3DCore::QBuffer(pitGeom);
                nb->setData(QByteArray(reinterpret_cast<const char*>(normals.constData()),
                    normals.size() * sizeof(float)));

                Qt3DCore::QAttribute* normAttr = new Qt3DCore::QAttribute(pitGeom);
                normAttr->setName(Qt3DCore::QAttribute::defaultNormalAttributeName());
                normAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
                normAttr->setVertexSize(3);
                normAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
                normAttr->setBuffer(nb);
                normAttr->setByteStride(3 * sizeof(float));
                normAttr->setCount(static_cast<uint>(pn * 2));
                pitGeom->addAttribute(normAttr);

                Qt3DCore::QBuffer* ib = new Qt3DCore::QBuffer(pitGeom);
                ib->setData(QByteArray(reinterpret_cast<const char*>(idx.constData()),
                    idx.size() * sizeof(quint32)));

                Qt3DCore::QAttribute* idxAttr = new Qt3DCore::QAttribute(pitGeom);
                idxAttr->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
                idxAttr->setVertexBaseType(Qt3DCore::QAttribute::UnsignedInt);
                idxAttr->setBuffer(ib);
                idxAttr->setCount(static_cast<uint>(idx.size()));
                pitGeom->addAttribute(idxAttr);

                pitRenderer->setGeometry(pitGeom);
                pitRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);

                Qt3DExtras::QPhongMaterial* mat = new Qt3DExtras::QPhongMaterial(pitEntity);
                mat->setDiffuse(color);
                mat->setAmbient(color.darker(150));
                mat->setShininess(5.0f);

                pitEntity->addComponent(pitRenderer);
                pitEntity->addComponent(mat);
            };

        // ── Pit lane surface (slightly lighter grey than main track) ──
        buildPitMesh(pit.edges.left, pit.edges.right, QColor(80, 80, 80));
        
        // ── Entry curve surface ───────────────────────────────────────
        // Build left/right edges for the bezier curve by offsetting perpendicular to travel
        auto buildCurveEdges = [&](const std::vector<QVector2D>& curve)
            -> std::pair<std::vector<QVector2D>, std::vector<QVector2D>>
            {
                float halfW = track->getTrackWidth() * 0.5f;
                std::vector<QVector2D> leftEdge, rightEdge;

                for (size_t i = 0; i < curve.size(); i++) {
                    QVector2D dir;
                    if (i == 0)
                        dir = (curve[1] - curve[0]).normalized();
                    else if (i == curve.size() - 1)
                        dir = (curve[i] - curve[i - 1]).normalized();
                    else
                        dir = ((curve[i] - curve[i - 1]) + (curve[i + 1] - curve[i])).normalized();

                    QVector2D normal = perpendicular(dir);
                    leftEdge.push_back(curve[i] + normal * halfW);
                    rightEdge.push_back(curve[i] - normal * halfW);
                }
                return { leftEdge, rightEdge };
            };
        
        // ── Entry/exit curve surfaces using edges already computed in Track ──
        buildPitMesh(pit.entryCurveEdges.left, pit.entryCurveEdges.right, QColor(80, 80, 80));
        buildPitMesh(pit.exitCurveEdges.left, pit.exitCurveEdges.right, QColor(80, 80, 80));

        //auto [entryLeft, entryRight] = buildCurveEdges(pit.entryCurve);
        //auto [exitLeft, exitRight] = buildCurveEdges(pit.exitCurve);
        //buildPitMesh(entryLeft, entryRight, QColor(80, 80, 80));
        //buildPitMesh(exitLeft, exitRight, QColor(80, 80, 80));

        qDebug() << "pit.edges.left:" << pit.edges.left.size();
        qDebug() << "pit.edges.right:" << pit.edges.right.size();
        qDebug() << "pit.centerLine:" << pit.centerLine.size();

        // ── Pit lane kerbs (same red/white pattern, narrower) ────────
        buildKerb(pit.edges.left, pit.centerLine, true);
        buildKerb(pit.edges.right, pit.centerLine, false);

            qDebug() << "Pit lane mesh built with" << pit.centerLine.size() << "points";
        }
    /*
    // ── Track walls (barriers) ───────────────────────────────
    // Simple vertical quads extruded upward from each edge
    float wallHeight = 8.0f;
    
	// We can reuse the same geometry building code for both left and right edges by passing the edge points and color as parameters
    auto buildWall = [&](const std::vector<QVector2D>& edge, QColor color) {
        Qt3DCore::QEntity* wallEntity = new Qt3DCore::QEntity(m_rootEntity);

        QVector<float>   wVerts;
        QVector<quint32> wIdx;
		QVector<float>   wNormals;
        wNormals.reserve(static_cast<int>(n) * 2 * 3);
        for (size_t i = 0; i < n * 2; i++) {
            wNormals << 0.0f << 0.0f << 1.0f;
        }
		// Each edge point generates 2 vertices (bottom and top), so we reserve space accordingly
        size_t en = edge.size();
        for (size_t i = 0; i < en; i++) {
            // Bottom vertex
            wVerts << edge[i].x() << 0.0f << edge[i].y();
            // Top vertex
            wVerts << edge[i].x() << wallHeight << edge[i].y();
        }
		// Indices: for each segment i, two triangles
        for (quint32 i = 0; i < static_cast<quint32>(en - 1); i++) {
			quint32 b0 = i * 2; //  bottom vertex of point i
			quint32 t0 = i * 2 + 1; //  top vertex of point i
			quint32 b1 = (i + 1) * 2; //  bottom vertex of point i+1
			quint32 t1 = (i + 1) * 2 + 1; //  top vertex of point i+1

            wIdx << b0 << b1 << t0;
            wIdx << t0 << b1 << t1;
        }

		// Create geometry for the wall same as we did for the track, but with our wall vertices and indices
        Qt3DRender::QGeometryRenderer* wRenderer = new Qt3DRender::QGeometryRenderer(wallEntity);
        //Qt3DCore::QGeometry* wGeom = new Qt3DCore::QGeometry(wallEntity);
        Qt3DCore::QGeometry* wGeom = new Qt3DCore::QGeometry(wRenderer);

        Qt3DCore::QBuffer* wVB = new Qt3DCore::QBuffer(wGeom);
        wVB->setData(QByteArray(reinterpret_cast<const char*>(wVerts.constData()),
            wVerts.size() * sizeof(float)));

        Qt3DCore::QAttribute* wPos = new Qt3DCore::QAttribute(wGeom);
        wPos->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
        wPos->setVertexBaseType(Qt3DCore::QAttribute::Float);
        wPos->setVertexSize(3);
        wPos->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
        wPos->setBuffer(wVB);
        wPos->setByteStride(3 * sizeof(float));
        wPos->setCount(static_cast<uint>(en * 2));
        wGeom->addAttribute(wPos);

        Qt3DCore::QBuffer* wIB = new Qt3DCore::QBuffer(wGeom);
        wIB->setData(QByteArray(reinterpret_cast<const char*>(wIdx.constData()),
            wIdx.size() * sizeof(quint32)));

        Qt3DCore::QAttribute* wIdxAttr = new Qt3DCore::QAttribute(wGeom);
        wIdxAttr->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
        wIdxAttr->setVertexBaseType(Qt3DCore::QAttribute::UnsignedInt);
        wIdxAttr->setBuffer(wIB);
        wIdxAttr->setCount(static_cast<uint>(wIdx.size()));
        wGeom->addAttribute(wIdxAttr);

        Qt3DCore::QBuffer* wNormalBuffer = new Qt3DCore::QBuffer(wGeom);
        wNormalBuffer->setData(QByteArray(reinterpret_cast<const char*>(wNormals.constData()),
            wNormals.size() * sizeof(float)));

        Qt3DCore::QAttribute* wNormAttr = new Qt3DCore::QAttribute(wGeom);
        wNormAttr->setName(Qt3DCore::QAttribute::defaultNormalAttributeName());
        wNormAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
        wNormAttr->setVertexSize(3);
        wNormAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
        wNormAttr->setBuffer(wNormalBuffer);
        wNormAttr->setByteStride(3 * sizeof(float));
        wNormAttr->setCount(static_cast<uint>(n * 2));
        wGeom->addAttribute(wNormAttr);

        //Qt3DRender::QGeometryRenderer* wRenderer = new Qt3DRender::QGeometryRenderer(wallEntity);
        wRenderer->setGeometry(wGeom);
        wRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);

		// Material with specified color and some shininess for the walls
        Qt3DExtras::QPhongMaterial* wMat = new Qt3DExtras::QPhongMaterial(wallEntity);
        wMat->setDiffuse(color);
        wMat->setAmbient(color.darker(150));

        wallEntity->addComponent(wRenderer);
        wallEntity->addComponent(wMat);
        };

	// Build left and right walls with different colors for better visibility
    buildWall(track->getTrackEdges().left, QColor(220, 50, 50));  // red left barrier
    buildWall(track->getTrackEdges().right, QColor(220, 50, 50));  // red right barrier
    */
	//debug info
    qDebug() << "Track mesh built with" << n << "segments (" << vertices.size() / 3
		<< "vertices, " << indices.size() / 3 << "triangles)";
}

// ─────────────────────────────────────────────
// Ground (grass outside the track)
// ─────────────────────────────────────────────
void Track3DViewer::buildGround()
{
	// Remove old ground entity if it exists
    if (m_groundEntity) {
        m_groundEntity->setParent(static_cast<Qt3DCore::QEntity*>(nullptr));
        delete m_groundEntity;
    }

    m_groundEntity = new Qt3DCore::QEntity(m_rootEntity);

	//  Create a large plane mesh for the ground
    Qt3DExtras::QPlaneMesh* planeMesh = new Qt3DExtras::QPlaneMesh();
    planeMesh->setWidth(4000.0f);
    planeMesh->setHeight(4000.0f);
    planeMesh->setMeshResolution(QSize(2, 2));

	//  Material with green colour for grass
    Qt3DExtras::QPhongMaterial* grassMat = new Qt3DExtras::QPhongMaterial(m_groundEntity);
    grassMat->setDiffuse(QColor(60, 140, 60));   // green grass
    grassMat->setAmbient(QColor(30, 80, 30));
    grassMat->setShininess(0.0f);

    // QPlaneMesh is in XZ plane by default, centred at origin – perfect
    Qt3DCore::QTransform* groundTransform = new Qt3DCore::QTransform(m_groundEntity);
	groundTransform->setTranslation(QVector3D(0, -1.0f, 0)); // slightly below track to avoid z-fighting with track surface

	//  Add components to the ground entity
    m_groundEntity->addComponent(planeMesh);
    m_groundEntity->addComponent(grassMat);
    m_groundEntity->addComponent(groundTransform);
}

// ─────────────────────────────────────────────
// Car placeholder
// ─────────────────────────────────────────────
/*
void Track3DViewer::buildCar()
{
    m_carEntity = new Qt3DCore::QEntity(m_rootEntity);

	// Car body (main box) for the moment – we can replace this with a more detailed model later
    Qt3DCore::QEntity* bodyEntity = new Qt3DCore::QEntity(m_carEntity);
    Qt3DExtras::QCuboidMesh* bodyMesh = new Qt3DExtras::QCuboidMesh();
    bodyMesh->setXExtent(18.0f);  // width
    bodyMesh->setYExtent(5.0f);   // height
    bodyMesh->setZExtent(30.0f);  // length

	// Material with red colour for the car body
    Qt3DExtras::QPhongMaterial* bodyMat = new Qt3DExtras::QPhongMaterial(bodyEntity);
    bodyMat->setDiffuse(QColor(220, 30, 30));  // red car
    bodyMat->setAmbient(QColor(120, 10, 10));
    bodyMat->setShininess(80.0f);

    bodyEntity->addComponent(bodyMesh);
    bodyEntity->addComponent(bodyMat);

    // Cabin (smaller box on top) 
    Qt3DCore::QEntity* cabinEntity = new Qt3DCore::QEntity(m_carEntity);
    Qt3DExtras::QCuboidMesh* cabinMesh = new Qt3DExtras::QCuboidMesh();
    cabinMesh->setXExtent(14.0f);
    cabinMesh->setYExtent(4.0f);
    cabinMesh->setZExtent(16.0f);

	// Cabin is positioned on top of the body, slightly towards the rear
    Qt3DCore::QTransform* cabinTransform = new Qt3DCore::QTransform(cabinEntity);
    cabinTransform->setTranslation(QVector3D(0, 4.5f, -2.0f));

	// Material with darker red for the cabin
    Qt3DExtras::QPhongMaterial* cabinMat = new Qt3DExtras::QPhongMaterial(cabinEntity);
    cabinMat->setDiffuse(QColor(180, 20, 20));
    cabinMat->setAmbient(QColor(90, 10, 10));

    cabinEntity->addComponent(cabinMesh);
    cabinEntity->addComponent(cabinMat);
    cabinEntity->addComponent(cabinTransform);

    // Transform for the whole car (updated each frame from Vehicule data)
    m_carTransform = new Qt3DCore::QTransform(m_carEntity);
    m_carTransform->setTranslation(QVector3D(0, 5.0f, 0));

    m_carEntity->addComponent(m_carTransform);
}
*/

//version 3d model .obj
void Track3DViewer::buildCar()
{
    // Parent entity - holds per-frame position/rotation
    m_carEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_carTransform = new Qt3DCore::QTransform(m_carEntity);
    m_carTransform->setTranslation(QVector3D(0, 5.0f, 0));
    m_carEntity->addComponent(m_carTransform);

    // Child entity - holds the scene loader
    Qt3DCore::QEntity* modelEntity = new Qt3DCore::QEntity(m_carEntity);

    Qt3DRender::QSceneLoader* loader = new Qt3DRender::QSceneLoader(modelEntity);
    loader->setSource(QUrl::fromLocalFile(
        QDir::currentPath() + "/3dModels/raceCarGreen.obj"
    ));

    // Optional: scale/reorient the model
    Qt3DCore::QTransform* modelTransform = new Qt3DCore::QTransform(modelEntity);
    modelTransform->setScale(5.0f);
    modelTransform->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, -90));

    modelEntity->addComponent(loader);
    modelEntity->addComponent(modelTransform);

    // Debug: print when loaded
    connect(loader, &Qt3DRender::QSceneLoader::statusChanged,
        [](Qt3DRender::QSceneLoader::Status status) {
            qDebug() << "Model status:" << status;
            // Ready = 2, Error = 3
        });
}



// ─────────────────────────────────────────────
// Helper – generic coloured box
// ─────────────────────────────────────────────
// Tool to create box entities with specified size, position, and color. Useful for debugging or simple objects.
Qt3DCore::QEntity* Track3DViewer::createBox(Qt3DCore::QEntity* parent,
    QVector3D size,
    QVector3D position,
    QColor color)
{
	// Create a new entity under the specified parent
    Qt3DCore::QEntity* entity = new Qt3DCore::QEntity(parent);

	// QCuboidMesh is a simple box mesh provided by Qt3D. We set its extents to define the size of the box.
    Qt3DExtras::QCuboidMesh* mesh = new Qt3DExtras::QCuboidMesh();
    mesh->setXExtent(size.x());
    mesh->setYExtent(size.y());
    mesh->setZExtent(size.z());

	// Transform to position the box in the world
    Qt3DCore::QTransform* transform = new Qt3DCore::QTransform(entity);
    transform->setTranslation(position);

	// Material with specified color
    Qt3DExtras::QPhongMaterial* mat = new Qt3DExtras::QPhongMaterial(entity);
    mat->setDiffuse(color);
    mat->setAmbient(color.darker(150));

	// Add components to the entity
    entity->addComponent(mesh);
    entity->addComponent(transform);
    entity->addComponent(mat);

    return entity;
}

void Track3DViewer::onUpdateFrame()
{
    if (m_vehicule)
        updateVehicule(m_vehicule);
}
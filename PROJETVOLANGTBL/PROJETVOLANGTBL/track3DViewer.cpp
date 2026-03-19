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
#include <QCoreApplication>
// for skybox
#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QTextureImage>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QTextureMaterial>

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
    //defaultFrameGraph()->setClearColor(QColor(135, 206, 235)); // light blue sky

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
    qDebug() << "setTrack called - checkpoints:" << track->getCheckpoints().size();
    buildTrackMesh(track);
	buildDecors(track);
    buildBezierWalls(track);
	buildCheckpoints(track);
    buildGround();
}
void Track3DViewer::updateVehicule(Vehicule* vehicule)
{
    m_vehicule = vehicule;
    if (!m_carTransform) return;

    float x = vehicule->getPosition().x();
    float y = vehicule->getPosition().y();
    float angle = vehicule->getAngle(); // en radian

    // Car stays at game logic position
    m_carTransform->setTranslation(QVector3D(x, 0.0f, y)); // 0 = on the ground
    QQuaternion rot = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0),
        -qRadiansToDegrees(angle));
    m_carTransform->setRotation(rot);

    // ── Smooth camera angle with lag ─────────────────────────
    if (m_firstPersonMode && m_camera) {

        // ── Smooth camera angle with lag ─────────────────────────
        float targetYaw = -qRadiansToDegrees(angle);

        // Normalize targetYaw to -180..180 range
        while (targetYaw > 180.0f) targetYaw -= 360.0f;
        while (targetYaw < -180.0f) targetYaw += 360.0f;

        // Normalize m_cameraYaw to -180..180 range
        while (m_cameraYaw > 180.0f) m_cameraYaw -= 360.0f;
        while (m_cameraYaw < -180.0f) m_cameraYaw += 360.0f;

        // Handle wrap-around difference
        float diff = targetYaw - m_cameraYaw;
        while (diff > 180.0f) diff -= 360.0f;
        while (diff < -180.0f) diff += 360.0f;

        // Smooth lerp
        m_cameraYaw += diff * (1.0f - m_cameraLag);

        // Convert smoothed yaw back to radians for position calculation
        float smoothAngle = qDegreesToRadians(-m_cameraYaw);

        // Camera position behind and above the car
        float camOffsetBack = 15.0f;
        float camHeight = 10.0f;

        float camX = x - camOffsetBack * qCos(smoothAngle);
        float camZ = y - camOffsetBack * qSin(smoothAngle);

        // Look ahead of the carw
        float lookX = x + 30.0f * qCos(smoothAngle);
        float lookZ = y + 30.0f * qSin(smoothAngle);

        m_camera->setPosition(QVector3D(camX, camHeight, camZ));
        m_camera->setViewCenter(QVector3D(lookX, 5.0f, lookZ));
        m_camera->setUpVector(QVector3D(0, 1, 0));
    }
    
    
}

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
    defaultFrameGraph()->setClearColor(Qt::transparent);
    // Disable frustum culling so skybox always renders
    defaultFrameGraph()->setFrustumCullingEnabled(false);
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

    // ── Key light (sun) - top right front ────────────────────
    Qt3DCore::QEntity* keyEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QDirectionalLight* keyLight = new Qt3DRender::QDirectionalLight(keyEntity);
    keyLight->setColor(QColor(255, 250, 240)); // warm white
	keyLight->setIntensity(0.4f); // main light old : 0.6f
    keyLight->setWorldDirection(QVector3D(-1.0f, -1.0f, 0.0).normalized());
    keyEntity->addComponent(keyLight);

    // ── Fill light - top left back ────────────────────────────
    Qt3DCore::QEntity* fillEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QDirectionalLight* fillLight = new Qt3DRender::QDirectionalLight(fillEntity);
    fillLight->setColor(QColor(150, 170, 255)); // cool blue fill
	fillLight->setIntensity(0.4f); // secondary light old : 0.3f
    fillLight->setWorldDirection(QVector3D(1.0f, -0.5f, 1.0f).normalized());
    fillEntity->addComponent(fillLight);

    // ── Back light - bottom up ────────────────────────────────
    Qt3DCore::QEntity* backEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QDirectionalLight* backLight = new Qt3DRender::QDirectionalLight(backEntity);
    backLight->setColor(QColor(200, 200, 200)); // neutral grey
	backLight->setIntensity(0.3f); // rim light old : 0.2f
    backLight->setWorldDirection(QVector3D(0.0f, 1.0f, 0.0f).normalized()); // from below
    backEntity->addComponent(backLight);

    // ── Car placeholder ──────────────────────────────────────
    buildCar();
	buildSkybox();
}


// ─────────────────────────────────────────────
// Skybox setup
// ─────────────────────────────────────────────

void Track3DViewer::buildSkybox()
{
    // Just use QSkyboxEntity - it works in Qt6 with correct path format
    Qt3DExtras::QSkyboxEntity* skybox = new Qt3DExtras::QSkyboxEntity(m_rootEntity);

    QString basePath = "file:///" + QDir::currentPath() + "/images/skybox/space/cubemap1";
    basePath.replace("\\", "/"); // fix Windows backslashes

    qDebug() << "Skybox base path:" << basePath;

    skybox->setBaseName(basePath);
    skybox->setExtension(".png");
    skybox->setGammaCorrectEnabled(false);

    
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
    //material->setDiffuse(QColor(60, 60, 60));    // dark grey = tarmac
    material->setDiffuse(QColor(241, 242, 246));  // Kenney's exact grey
	material->setAmbient(QColor(0, 0, 0)); // ambient is usually darker than diffuse
    material->setShininess(0.0f);

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
        float kerbHeight = 0.05f;
        float segmentLength = 10.0f;
        float accumulated = 0.0f;
        bool isRed = true;

        if(flipWinding) {
            // If we flip winding, we start with white instead of red to maintain the alternating pattern
			kerbHeight = 0.15f;
		}

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
                
                //Qt3DExtras::QPhongMaterial* trackMat = new Qt3DExtras::QPhongMaterial(m_trackEntity);
                //QColor trackColor(241, 242, 246);
                //trackMat->setDiffuse(trackColor);
                //trackMat->setAmbient(trackColor.darker(200)); // 50% darker
                //trackMat->setSpecular(QColor(0, 0, 0));
                //trackMat->setShininess(0.0f);
                //m_trackEntity->addComponent(trackMat);
                entity->addComponent(renderer);
                entity->addComponent(mat);
            };

        // 2 entities per edge instead of hundreds
        makeEntity(redVerts, redNormals, redIdx, QColor(220, 30, 30));
        makeEntity(whiteVerts, whiteNormals, whiteIdx, Qt::white);
        };

    
		buildKerb(track->getTrackEdges().left, track->getCenterLine(), true); // left kerb with normal winding
		buildKerb(track->getTrackEdges().right, track->getCenterLine(), false); // right kerb with flipped winding to keep normals facing up

    // ───────────────────────────────────────── ─────────────────────────────────────────
    // ── Pit lane 3D mesh ─────────────────────────────────────────
    // ───────────────────────────────────────── ─────────────────────────────────────────
        

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
                verts << left[i].x() << 0.1f << left[i].y();
            for (size_t i = 0; i < pn; i++)
                verts << right[i].x() << 0.1f << right[i].y();

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
            mat->setAmbient(QColor(0,0,0));
            mat->setShininess(0.0f);
            
            //Qt3DExtras::QPhongMaterial* pitMat = new Qt3DExtras::QPhongMaterial(m_trackEntity);
            //QColor pitColor(color); // Kenney's grey
            //pitMat->setDiffuse(pitColor);
            //pitMat->setAmbient(pitColor.darker(200)); // 50% darker
            //pitMat->setSpecular(QColor(0, 0, 0));
            //pitMat->setShininess(0.0f);
            //m_trackEntity->addComponent(pitMat);
            pitEntity->addComponent(pitRenderer);
            pitEntity->addComponent(mat);
        };
       
    if (track->hasPitLane()) {
        PitLane pit = track->getPitLane();

        // ── Build one continuous left and right edge array ──────────
        // entry curve → pit straight → exit curve = no seams, no overlap
        std::vector<QVector2D> fullLeft, fullRight;

        // Left side: entry curve left → pit straight left → exit curve left
        for (const auto& p : pit.entryCurveEdges.left)  fullLeft.push_back(p);
        for (const auto& p : pit.edges.left)             fullLeft.push_back(p);
        for (const auto& p : pit.exitCurveEdges.left)    fullLeft.push_back(p);

        // Right side: same order
        for (const auto& p : pit.entryCurveEdges.right)  fullRight.push_back(p);
        for (const auto& p : pit.edges.right)             fullRight.push_back(p);
        for (const auto& p : pit.exitCurveEdges.right)    fullRight.push_back(p);

        // ── Remove duplicate junction points to avoid degenerate triangles ──
        // The last point of entryCurveEdges == first point of edges, so remove duplicate
        if (!fullLeft.empty() && fullLeft.size() > pit.entryCurveEdges.left.size()) {
            fullLeft.erase(fullLeft.begin() + pit.entryCurveEdges.left.size());
            fullRight.erase(fullRight.begin() + pit.entryCurveEdges.right.size());
        }
        //// Same for the junction between straight and exit curve
        size_t exitJunction = pit.entryCurveEdges.left.size() - 1 + pit.edges.left.size();
        if (exitJunction < fullLeft.size()) {
            fullLeft.erase(fullLeft.begin() + exitJunction);
            fullRight.erase(fullRight.begin() + exitJunction);
        }

        // ── Single mesh - no z-fighting, no gaps ────────────────────
        //buildPitMesh(fullLeft, fullRight, QColor(60, 60, 60));
		buildPitMesh(fullLeft, fullRight, QColor(241, 242, 246)); // Kenney's exact grey
        //material->setDiffuse(QColor(60, 60, 60));    // dark grey = tarmac
        //material->setAmbient(QColor(40, 40, 40)); // ambient is usually darker than diffuse
        //material->setShininess(5.0f);

        // ── Kerbs only on the straight section ──────────────────────
        //if (pit.edges.left.size() == pit.centerLine.size())
        //    buildKerb(pit.edges.left, pit.centerLine, true);
        //if (pit.edges.right.size() == pit.centerLine.size())
        //    buildKerb(pit.edges.right, pit.centerLine, false);

        qDebug() << "Pit lane mesh built -"
            << "entry:" << pit.entryCurveEdges.left.size()
            << "straight:" << pit.edges.left.size()
            << "exit:" << pit.exitCurveEdges.left.size()
            << "total:" << fullLeft.size();
            
    }
}

// ─────────────────────────────────────────────
// Ground (grass outside the track)
// ─────────────────────────────────────────────

void Track3DViewer::buildGround()
{
    if (m_groundEntity) {
        m_groundEntity->setParent(static_cast<Qt3DCore::QEntity*>(nullptr));
        delete m_groundEntity;
    }

    m_groundEntity = new Qt3DCore::QEntity(m_rootEntity);

    // ── Build a tessellated plane with procedural bumps ──────────────
    const int   GRID = 120;       // grid subdivisions (higher = smoother bumps)
    const float SIZE = 2000.0f;
    const float STEP = SIZE / GRID;
    const float BUMP = 2.8f;      // max bump height
    const float FREQ1 = 0.018f;    // large rolling hills
    const float FREQ2 = 0.055f;    // medium lumps
    const float FREQ3 = 0.130f;    // small pebble texture

    int   vertCount = (GRID + 1) * (GRID + 1);
    int   idxCount = GRID * GRID * 6;

    QVector<float>   verts;   verts.reserve(vertCount * 3);
    QVector<float>   normals; normals.reserve(vertCount * 3);
    QVector<quint32> idx;     idx.reserve(idxCount);

    // Height function - layered sine waves for organic bumps
    auto height = [&](float x, float z) -> float {
        return BUMP * (
            0.50f * sinf(x * FREQ1 + 1.3f) * cosf(z * FREQ1 * 0.8f + 0.7f) +
            0.30f * sinf(x * FREQ2 + 2.1f) * sinf(z * FREQ2 + 1.5f) +
            0.20f * cosf(x * FREQ3 + 0.4f) * cosf(z * FREQ3 * 1.2f + 3.1f)
            );
        };

    // Generate vertices
    for (int row = 0; row <= GRID; ++row) {
        for (int col = 0; col <= GRID; ++col) {
            float x = -SIZE * 0.5f + col * STEP;
            float z = -SIZE * 0.5f + row * STEP;
            float y = height(x, z);
            verts << x << y << z;
        }
    }

    // Compute smooth normals via central differences
    auto idx2d = [&](int r, int c) { return r * (GRID + 1) + c; };
    for (int row = 0; row <= GRID; ++row) {
        for (int col = 0; col <= GRID; ++col) {
            int   r0 = qMax(row - 1, 0), r1 = qMin(row + 1, GRID);
            int   c0 = qMax(col - 1, 0), c1 = qMin(col + 1, GRID);
            float hL = verts[idx2d(row, c0) * 3 + 1];
            float hR = verts[idx2d(row, c1) * 3 + 1];
            float hD = verts[idx2d(r0, col) * 3 + 1];
            float hU = verts[idx2d(r1, col) * 3 + 1];
            QVector3D n = QVector3D(hL - hR, 2.0f * STEP, hD - hU).normalized();
            normals << n.x() << n.y() << n.z();
        }
    }

    // Generate indices
    for (int row = 0; row < GRID; ++row) {
        for (int col = 0; col < GRID; ++col) {
            quint32 tl = idx2d(row, col);
            quint32 tr = idx2d(row, col + 1);
            quint32 bl = idx2d(row + 1, col);
            quint32 br = idx2d(row + 1, col + 1);
            idx << tl << bl << tr;
            idx << tr << bl << br;
        }
    }

    // ── Wire up Qt3D geometry ────────────────────────────────────────
    Qt3DRender::QGeometryRenderer* renderer = new Qt3DRender::QGeometryRenderer(m_groundEntity);
    Qt3DCore::QGeometry* geom = new Qt3DCore::QGeometry(renderer);

    auto makeAttr = [&](Qt3DCore::QBuffer* buf, const QString& name,
        uint count, Qt3DCore::QAttribute::AttributeType type
        = Qt3DCore::QAttribute::VertexAttribute)
        {
            Qt3DCore::QAttribute* a = new Qt3DCore::QAttribute(geom);
            a->setName(name);
            a->setVertexBaseType(Qt3DCore::QAttribute::Float);
            a->setVertexSize(3);
            a->setAttributeType(type);
            a->setBuffer(buf);
            a->setByteStride(3 * sizeof(float));
            a->setCount(count);
            geom->addAttribute(a);
            return a;
        };

    Qt3DCore::QBuffer* vb = new Qt3DCore::QBuffer(geom);
    vb->setData(QByteArray(reinterpret_cast<const char*>(verts.constData()),
        verts.size() * sizeof(float)));
    makeAttr(vb, Qt3DCore::QAttribute::defaultPositionAttributeName(),
        static_cast<uint>(vertCount));

    Qt3DCore::QBuffer* nb = new Qt3DCore::QBuffer(geom);
    nb->setData(QByteArray(reinterpret_cast<const char*>(normals.constData()),
        normals.size() * sizeof(float)));
    makeAttr(nb, Qt3DCore::QAttribute::defaultNormalAttributeName(),
        static_cast<uint>(vertCount));

    Qt3DCore::QBuffer* ib = new Qt3DCore::QBuffer(geom);
    ib->setData(QByteArray(reinterpret_cast<const char*>(idx.constData()),
        idx.size() * sizeof(quint32)));
    Qt3DCore::QAttribute* ia = new Qt3DCore::QAttribute(geom);
    ia->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
    ia->setVertexBaseType(Qt3DCore::QAttribute::UnsignedInt);
    ia->setBuffer(ib);
    ia->setCount(static_cast<uint>(idx.size()));
    geom->addAttribute(ia);

    renderer->setGeometry(geom);
    renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);

    Qt3DExtras::QPhongMaterial* mat = new Qt3DExtras::QPhongMaterial(m_groundEntity);
    mat->setDiffuse(QColor(119, 171, 86));
    mat->setAmbient(QColor(30, 80, 30));
    mat->setShininess(0.0f);

    //Qt3DExtras::QPhongMaterial* grassMat = new Qt3DExtras::QPhongMaterial(m_groundEntity);
    //grassMat->setDiffuse(QColor(80, 200, 80));    // bright cartoon green
    //grassMat->setAmbient(QColor(60, 180, 60));    // close to diffuse = flat shading look
    //grassMat->setSpecular(QColor(0, 0, 0));       // no specular = no shine
    //grassMat->setShininess(0.0f);

    Qt3DCore::QTransform* t = new Qt3DCore::QTransform(m_groundEntity);
    t->setTranslation(QVector3D(0, -1.0f, 0));

    m_groundEntity->addComponent(renderer);
    m_groundEntity->addComponent(mat);
    m_groundEntity->addComponent(t);
}
/*
void Track3DViewer::buildGround()
{
	 //Remove old ground entity if it exists
    if (m_groundEntity) {
        m_groundEntity->setParent(static_cast<Qt3DCore::QEntity*>(nullptr));
        delete m_groundEntity;
    }

    m_groundEntity = new Qt3DCore::QEntity(m_rootEntity);

	//  Create a large plane mesh for the ground
    Qt3DExtras::QPlaneMesh* planeMesh = new Qt3DExtras::QPlaneMesh();
	// world dimensions - large enough to cover the whole track and surroundings
    planeMesh->setWidth(2000.0f);
    planeMesh->setHeight(2000.0f); 
    planeMesh->setMeshResolution(QSize(2, 2));

	//  Material with green colour for grass
    Qt3DExtras::QPhongMaterial* grassMat = new Qt3DExtras::QPhongMaterial(m_groundEntity);
    grassMat->setDiffuse(QColor(119, 171, 86));   // Kenney's signature green
    grassMat->setAmbient(QColor(30, 80, 30));
    grassMat->setShininess(0.0f);
    //Qt3DExtras::QPhongMaterial* grassMat = new Qt3DExtras::QPhongMaterial(m_groundEntity);
    //QColor grassColor(119, 171, 86);
    //grassMat->setDiffuse(grassColor);
    //grassMat->setAmbient(grassColor.darker(200));
    //grassMat->setSpecular(QColor(0, 0, 0));
    //grassMat->setShininess(0.0f);
	
    // QPlaneMesh is in XZ plane by default, centred at origin – perfect
    Qt3DCore::QTransform* groundTransform = new Qt3DCore::QTransform(m_groundEntity);
	groundTransform->setTranslation(QVector3D(0, -1.0f, 0)); // slightly below track to avoid z-fighting with track surface

	//  Add components to the ground entity
    m_groundEntity->addComponent(planeMesh);
    m_groundEntity->addComponent(grassMat);
    m_groundEntity->addComponent(groundTransform);
}
*/
void Track3DViewer::buildCheckpoints(Track* track)
{
    for (Qt3DCore::QEntity* e : m_checkpointEntities) {
        e->setParent(static_cast<Qt3DCore::QEntity*>(nullptr));
        delete e;
    }
    m_checkpointEntities.clear();

    if (!track) return;

    const auto& cps = track->getCheckpoints();
    
    for (int i = 0; i < (int)cps.size(); i++) {
        const CheckpointData& cp = cps[i];
        
        Qt3DCore::QEntity* cpEntity = new Qt3DCore::QEntity(m_rootEntity);

        // Position at center between left and right edge
        QVector2D center2D = (cp.left + cp.right) / 2.0f;
        
        
		// Calculate forward direction from left to right edge and derive rotation angle
        QVector3D fwdAxis(cp.forward.x(), 0.0f, cp.forward.y());
        QVector3D upAxis(0.0f, 1.0f, 0.0f);
        QVector3D rightAxis = QVector3D::crossProduct(upAxis, fwdAxis).normalized();

        // Transform
        Qt3DCore::QTransform* transform = new Qt3DCore::QTransform(cpEntity);
        //transform->setTranslation(QVector3D(center2D.x(), 0.0f, center2D.y()));
        transform->setTranslation(QVector3D(center2D.x(), 0.0f, center2D.y()));
		transform->setRotation(Qt3DCore::QTransform::fromAxes(rightAxis, upAxis, fwdAxis));
        //transform->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, angle));
        //transform->setRotationY(angle);
        float scale = 40.0f; // base scale to match your .dae model size (tune as needed) 
		transform->setScale(40.0f);
		//qDebug() << "Checkpoint" << i << "transform:" << transform->translation() << transform->rotationY();
        cpEntity->addComponent(transform);

        // Offset entity to shift model's left-corner origin to center
        Qt3DCore::QEntity* offsetEntity = new Qt3DCore::QEntity(cpEntity);
        Qt3DCore::QTransform* offsetTransform = new Qt3DCore::QTransform(offsetEntity);

        
        // Starting line model does not have the same center
        if (i == 0) {
            offsetTransform->setTranslation(QVector3D(-1.0f / 2, 0.0f, 0.0f));
        }
        // model dimensions : x = 1.87f , y = 0.93f , z = 0.33f for every other checkpoint
        else {
            offsetTransform->setTranslation(QVector3D(-1.87f / 2, 0.0f, -0.33f / 2));
        }
        
        
        offsetEntity->addComponent(offsetTransform);

        // Attach loader to offsetEntity instead of directly to cpEntity
        Qt3DCore::QEntity* modelEntity = new Qt3DCore::QEntity(offsetEntity); // <-- parent changed
        
        Qt3DRender::QSceneLoader* loader = new Qt3DRender::QSceneLoader(modelEntity);

        connect(loader, &Qt3DRender::QSceneLoader::statusChanged,
            [modelEntity, i](Qt3DRender::QSceneLoader::Status status) {
                qDebug() << "Checkpoint" << i << "status:" << status;
                // 0=None, 1=Loading, 2=Ready, 3=Error
                if (status == Qt3DRender::QSceneLoader::Error) {
                    qDebug() << "Checkpoint" << i << "FAILED TO LOAD";
                    return;
                }
                if (status != Qt3DRender::QSceneLoader::Ready) return;

                QList<Qt3DExtras::QPhongMaterial*> mats =
                    modelEntity->findChildren<Qt3DExtras::QPhongMaterial*>();
                qDebug() << "Checkpoint" << i << "loaded -" << mats.size() << "materials";

                for (Qt3DExtras::QPhongMaterial* mat : mats) {
                    mat->setShininess(0.0f);
                    mat->setSpecular(QColor(0, 0, 0));
                    QColor diff = mat->diffuse();
                    mat->setAmbient(diff.darker(200));
                }
            });

        modelEntity->addComponent(loader);

        QString modelPath;
        // Load .dae model
        if (i == 0) {
            modelPath = QDir::currentPath() + "/3dModels/dae/overheadRound.dae"; // Starting line
        }
        else {
            modelPath = QDir::currentPath() + "/3dModels/dae/overheadRoundColored.dae"; // Checkpoints
        }
        
        //qDebug() << "Loading checkpoint from:" << modelPath;
        loader->setSource(QUrl::fromLocalFile(modelPath));

        m_checkpointEntities.push_back(cpEntity);
    }
}

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
        QDir::currentPath() + "/3dModels/dae/raceCarGreen.dae"
    ));
   
   
    // Optional: scale/reorient the model
    Qt3DCore::QTransform* modelTransform = new Qt3DCore::QTransform(modelEntity);
    modelTransform->setScale(5.0f);
    modelTransform->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, -90));

    modelEntity->addComponent(loader);
    modelEntity->addComponent(modelTransform);

    //// Debug: print when loaded
    //connect(loader, &Qt3DRender::QSceneLoader::statusChanged,
    //    [](Qt3DRender::QSceneLoader::Status status) {
    //        qDebug() << "Model status:" << status;
    //        // Ready = 2, Error = 3
    //    });
    connect(loader, &Qt3DRender::QSceneLoader::statusChanged,
        [modelEntity](Qt3DRender::QSceneLoader::Status status) {
            qDebug() << "Model status:" << status;
            if (status != Qt3DRender::QSceneLoader::Ready) return;

            // Make car materials flat/cartoon like Kenney style
            QList<Qt3DExtras::QPhongMaterial*> mats =
                modelEntity->findChildren<Qt3DExtras::QPhongMaterial*>();

            for (Qt3DExtras::QPhongMaterial* mat : mats) {
                mat->setShininess(0.0f);
                mat->setSpecular(QColor(0, 0, 0));

                // Boost ambient to 80% of diffuse for flat cartoon look
                QColor diff = mat->diffuse();
                mat->setAmbient(QColor(
                    diff.red() * 0.8f,
                    diff.green() * 0.8f,
                    diff.blue() * 0.8f
                ));
            }

            qDebug() << "Car materials flattened -" << mats.size() << "materials found";
        });
}

//------------------------------------------
//--- buildDecor - version 3d model .obj ---
//------------------------------------------
void Track3DViewer::buildDecors(Track* track)
{
    // Remove old decor entities if they exist
    for (Qt3DCore::QEntity* e : m_decorEntities) {
        e->setParent(static_cast<Qt3DCore::QEntity*>(nullptr));
        delete e;
    }
    m_decorEntities.clear();

    if (!track) return;

    const auto& decors = track->getDecors();
    if (decors.empty()) return;

    for (DecorPieces* decor : decors) {
        if (!decor) continue;

        // Parent entity for this decor - holds position/rotation
        Qt3DCore::QEntity* decorEntity = new Qt3DCore::QEntity(m_rootEntity);

        // Transform: position from 2D world (x,y) → 3D (x, 0, y)
        Qt3DCore::QTransform* decorTransform = new Qt3DCore::QTransform(decorEntity);
        float x = decor->getInfo().pos.x();
        float y = decor->getInfo().pos.y();
        float angle = decor->getInfo().angle;

        decorTransform->setTranslation(QVector3D(x, 0.0f, y));
        decorTransform->setRotation(
            QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0),
                -qRadiansToDegrees(angle)));
        decorEntity->addComponent(decorTransform);

        // Child entity holds the actual 3D model
        Qt3DCore::QEntity* modelEntity = new Qt3DCore::QEntity(decorEntity);

        // Add to buildDecors() before creating loader:
        qDebug() << "App dir:" << QCoreApplication::applicationDirPath();
        QDir sceneDir(QCoreApplication::applicationDirPath() + "/sceneparsers");
        qDebug() << "Sceneparsers exists:" << sceneDir.exists();
        qDebug() << "Files:" << sceneDir.entryList(QDir::Files);
        Qt3DRender::QSceneLoader* loader = new Qt3DRender::QSceneLoader(modelEntity);
        QString modelPath = QDir::currentPath() + decor->getInfo().modelPath;
        loader->setSource(QUrl::fromLocalFile(modelPath));

       
        // Scale based on decor info
        Qt3DCore::QTransform* modelTransform = new Qt3DCore::QTransform(modelEntity);
        modelTransform->setScale3D(QVector3D(
            decor->getInfo().width * decor->getScale(),  // match world units
            decor->getInfo().height * decor->getScale(),
            decor->getInfo().length * decor->getScale()));
        modelEntity->addComponent(loader);
        modelEntity->addComponent(modelTransform);

        //// Debug when each model loads
        //connect(loader, &Qt3DRender::QSceneLoader::statusChanged,
        //    [modelPath](Qt3DRender::QSceneLoader::Status status) {
        //        qDebug() << "Decor model" << modelPath << "status:" << status;
        //    });
        connect(loader, &Qt3DRender::QSceneLoader::statusChanged,
            [modelEntity](Qt3DRender::QSceneLoader::Status status) {
                if (status != Qt3DRender::QSceneLoader::Ready) return;

                QList<Qt3DExtras::QPhongMaterial*> mats =
                    modelEntity->findChildren<Qt3DExtras::QPhongMaterial*>();

                for (Qt3DExtras::QPhongMaterial* mat : mats) {
                    mat->setShininess(0.0f);
                    mat->setSpecular(QColor(0, 0, 0));
                    QColor diff = mat->diffuse();
                    mat->setAmbient(QColor(
                        diff.red() * 0.8f,
                        diff.green() * 0.8f,
                        diff.blue() * 0.8f
                    ));
                }
            });
        m_decorEntities.push_back(decorEntity);
    }

    qDebug() << "Built" << m_decorEntities.size() << "decor entities";
}


//------------------------------------------
//--- buildWalls - version 3d model .dae ---
//------------------------------------------
void Track3DViewer::buildBezierWalls(Track* track)
{
    // Clear old wall entities
    for (Qt3DCore::QEntity* e : m_wallEntities) {
        e->setParent(static_cast<Qt3DCore::QEntity*>(nullptr));
        delete e;
    }
    m_wallEntities.clear();

    if (!track || !track->hasBezierCurves()) return;

    const auto& curves = track->getBezierCurves();

    // Helper to evaluate bezier point
    auto evalBezier = [](const BezierCurveData& c, float t) -> QVector3D {
        float u = 1.0f - t;
        QVector2D p = c.p0 * (u * u * u)
            + c.p1 * (3 * u * u * t)
            + c.p2 * (3 * u * t * t)
            + c.p3 * (t * t * t);
        return QVector3D(p.x(), 0.0f, p.y());
        };
	float scale = 20.0f; 
    int   segmentsPerCurve = 20;    // how many wall pieces per curve
    float wallModelLength = 0.12 * scale; // length of your .dae wall model along Z

    for (const BezierCurveData& curve : curves) {
        for (int i = 0; i < segmentsPerCurve; i++) {
            float t0 = (float)i / segmentsPerCurve;
            float t1 = (float)(i + 1) / segmentsPerCurve;

            QVector3D p0 = evalBezier(curve, t0);
            QVector3D p1 = evalBezier(curve, t1);

            // Segment length and direction
            QVector3D dir = (p1 - p0);
            float     segLen = dir.length();
            QVector3D dirN = dir.normalized();

            // Center position of this segment
            QVector3D center = (p0 + p1) / 2.0f;

            // ── Create wall entity ───────────────────────────
            Qt3DCore::QEntity* wallEntity = new Qt3DCore::QEntity(m_rootEntity);

            Qt3DCore::QTransform* wallTransform = new Qt3DCore::QTransform(wallEntity);

            // Position at segment center
            wallTransform->setTranslation(center);

            // Rotate to align x axis with segment direction
            QQuaternion rot = QQuaternion::rotationTo(
                QVector3D(1, 0, 0), // model faces X
                dirN                // target direction
            );
            wallTransform->setRotation(rot);

            // Scale Z to match segment length
            // Z and Y stay at 1.0 to keep wall proportions
            wallTransform->setScale3D(QVector3D(
                segLen / wallModelLength * scale,
                1.0f * scale,
                1.0f * scale // stretch X to fit segment
            ));

            wallEntity->addComponent(wallTransform);

            // ── Load .dae model ──────────────────────────────
            Qt3DCore::QEntity* modelEntity = new Qt3DCore::QEntity(wallEntity);
            Qt3DRender::QSceneLoader* loader =
                new Qt3DRender::QSceneLoader(modelEntity);

            loader->setSource(QUrl::fromLocalFile(
                QDir::currentPath() + "/3dModels/dae/barrierWhite.dae"));

            //connect(loader, &Qt3DRender::QSceneLoader::statusChanged,
            //    [i](Qt3DRender::QSceneLoader::Status status) {
            //        if (status == Qt3DRender::QSceneLoader::Error)
            //            qDebug() << "Wall segment" << i << "failed to load";
            //    });
            connect(loader, &Qt3DRender::QSceneLoader::statusChanged,
                [modelEntity](Qt3DRender::QSceneLoader::Status status) {
                    if (status != Qt3DRender::QSceneLoader::Ready) return;

                    QList<Qt3DExtras::QPhongMaterial*> mats =
                        modelEntity->findChildren<Qt3DExtras::QPhongMaterial*>();

                    for (Qt3DExtras::QPhongMaterial* mat : mats) {
                        mat->setShininess(0.0f);
                        mat->setSpecular(QColor(0, 0, 0));
                        QColor diff = mat->diffuse();
                        mat->setAmbient(QColor(
                            diff.red() * 0.8f,
                            diff.green() * 0.8f,
                            diff.blue() * 0.8f
                        ));
                    }
                });
            modelEntity->addComponent(loader);
            m_wallEntities.push_back(wallEntity);
        }
    }

    qDebug() << "Built" << m_wallEntities.size() << "wall segments";
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
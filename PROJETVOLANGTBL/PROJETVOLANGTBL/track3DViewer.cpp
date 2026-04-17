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
#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QTextureImage>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QTextureMaterial>

#include <QColor>
#include <QtMath>
#include <QTimer>

Track3DViewer::Track3DViewer(QScreen* screen)
    : Qt3DExtras::Qt3DWindow(screen)
{
    m_rootEntity = new Qt3DCore::QEntity();
    setRootEntity(m_rootEntity);

    
}

Track3DViewer::~Track3DViewer() {}

void Track3DViewer::setTrack(Track* track)
{
    m_track = track;

    if (m_sceneRoot) {
        m_sceneRoot->setEnabled(false);
        m_sceneRoot->setParent(static_cast<Qt3DCore::QEntity*>(nullptr));
        delete m_sceneRoot;
        m_sceneRoot = nullptr;
    }

    m_orbitController = nullptr;
    m_fpController    = nullptr;
    m_skybox          = nullptr;
    m_skyTransform    = nullptr;
    m_trackEntity     = nullptr;
    m_carEntity       = nullptr;
    m_carTransform    = nullptr;
    m_groundEntity    = nullptr;
    m_decorEntities.clear();
    m_checkpointEntities.clear();
    m_wallEntities.clear();
    m_instancedDecorEntities.clear();
    m_loaderCache.clear();

    m_sceneRoot = new Qt3DCore::QEntity(m_rootEntity);

    buildScene(track);
    buildTrackMesh(track);
    buildBezierWalls(track);
	buildCheckpoints(track);
    buildGround(track);
    buildInstancedDecors(track);
}
void Track3DViewer::updateVehicule(Vehicule* vehicule)
{
    m_vehicule = vehicule;
    if (!m_carTransform) return;

    float x = vehicule->getPosition().x();
    float y = vehicule->getPosition().y();
    float angle = vehicule->getAngle(); // en radian

    const float LOAD_RADIUS = 500.0f;

    for (Qt3DCore::QEntity* e : m_decorEntities) {
        Qt3DCore::QTransform* t = e->findChild<Qt3DCore::QTransform*>();
        if (!t) continue;
        float dx = t->translation().x() - x;
        float dz = t->translation().z() - y;
        float dist = std::sqrt(dx * dx + dz * dz);
        e->setEnabled(dist < LOAD_RADIUS); 
    }

    m_carTransform->setTranslation(QVector3D(x, 0.0f, y));
    QQuaternion rot = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0),
        -qRadiansToDegrees(angle));
    m_carTransform->setRotation(rot);

    if (m_firstPersonMode && m_camera) {

        float targetYaw = -qRadiansToDegrees(angle);

        while (targetYaw > 180.0f) targetYaw -= 360.0f;
        while (targetYaw < -180.0f) targetYaw += 360.0f;

        while (m_cameraYaw > 180.0f) m_cameraYaw -= 360.0f;
        while (m_cameraYaw < -180.0f) m_cameraYaw += 360.0f;

        float diff = targetYaw - m_cameraYaw;
        while (diff > 180.0f) diff -= 360.0f;
        while (diff < -180.0f) diff += 360.0f;

        m_cameraYaw += diff * (1.0f - m_cameraLag);

        float smoothAngle = qDegreesToRadians(-m_cameraYaw);

        float camOffsetBack = 0.0f;
        float camHeight = 0.0f;

        if (carCamthird) {

            camOffsetBack = 15.0f;
            camHeight = 10.0f;
        }
        if (!carCamthird) {

            camOffsetBack = 0.5f;
            camHeight = 2.0f;
        }
		

        float camX = x - camOffsetBack * qCos(smoothAngle);
        float camZ = y - camOffsetBack * qSin(smoothAngle);

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
        m_fpController->setEnabled(false); 
}

void Track3DViewer::changeCameraMode()
{
    if (carCamthird) {
        carCamthird = false; 
    } else {
        carCamthird = true; 
	}
}

bool Track3DViewer::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_V) changeCameraMode();
    }
    return QObject::eventFilter(obj, event);
}

void Track3DViewer::buildScene(Track* track)
{
    defaultFrameGraph()->setClearColor(Qt::transparent);

    defaultFrameGraph()->setFrustumCullingEnabled(false);
    // ── Camera
    m_camera = camera();
	m_camera->lens()->setPerspectiveProjection(70.0f, 16.0f / 9.0f, 0.1f, 5000.0f); 
	m_camera->setPosition(QVector3D(0, 300, 300)); 
	m_camera->setViewCenter(QVector3D(0, 0, 0));
	m_camera->setUpVector(QVector3D(0, 1, 0)); 

    m_orbitController = new Qt3DExtras::QOrbitCameraController(m_sceneRoot);
    m_orbitController->setCamera(m_camera);
    m_orbitController->setLinearSpeed(300.0f);
    m_orbitController->setLookSpeed(180.0f);
    m_orbitController->setEnabled(!m_firstPersonMode);

    buildLights();

    buildCar();
	buildSkybox(track);

    Qt3DRender::QFrameGraphNode* fg = defaultFrameGraph();
    QList<Qt3DRender::QFilterKey*> keys =
        fg->findChildren<Qt3DRender::QFilterKey*>();
}

void Track3DViewer::buildLights()
{
    if (m_lightsBuilt) return;

    Qt3DCore::QEntity* keyEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QDirectionalLight* keyLight = new Qt3DRender::QDirectionalLight(keyEntity);
    keyLight->setColor(QColor(255, 250, 240));
    keyLight->setIntensity(0.4f); 
    keyLight->setWorldDirection(QVector3D(-1.0f, -1.0f, 0.0).normalized());
    keyEntity->addComponent(keyLight);

    Qt3DCore::QEntity* fillEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QDirectionalLight* fillLight = new Qt3DRender::QDirectionalLight(fillEntity);
    fillLight->setColor(QColor(150, 170, 255));
    fillLight->setIntensity(0.4f); 
    fillLight->setWorldDirection(QVector3D(1.0f, -0.5f, 1.0f).normalized());
    fillEntity->addComponent(fillLight);

    Qt3DCore::QEntity* backEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QDirectionalLight* backLight = new Qt3DRender::QDirectionalLight(backEntity);
    backLight->setColor(QColor(200, 200, 200)); 
    backLight->setIntensity(0.3f); 
    backLight->setWorldDirection(QVector3D(0.0f, -1.0f, 0.0f).normalized()); 
    backEntity->addComponent(backLight);

    m_lightsBuilt = true;
}

void Track3DViewer::buildSkybox(Track* track)
{
    
    Qt3DExtras::QSkyboxEntity* skybox = new Qt3DExtras::QSkyboxEntity(m_sceneRoot);

    QString basePath = "file:///" + QDir::currentPath() + track->getCurrentChoixMapData().skyboxFilePath;
    basePath.replace("\\", "/"); 
    skybox->setBaseName(basePath);
    skybox->setExtension(".png");
    skybox->setGammaCorrectEnabled(false);

    
}

void Track3DViewer::buildTrackMesh(Track* track)
{

    const auto& left = track->getTrackEdges().left;
    const auto& right = track->getTrackEdges().right;

    if (left.size() < 2 || right.size() < 2) return;

    size_t n = qMin(left.size(), right.size());

    QVector<float>   vertices;
    QVector<quint32> indices;
    vertices.reserve(static_cast<int>(n) * 2 * 3);
    indices.reserve(static_cast<int>(n - 1) * 6);

    for (size_t i = 0; i < n; i++) vertices << left[i].x() << 0.0f << left[i].y();
    for (size_t i = 0; i < n; i++) vertices << right[i].x() << 0.0f << right[i].y();

    QVector<float> normals;
    normals.reserve(static_cast<int>(n) * 2 * 3);
    for (size_t i = 0; i < n * 2; i++) normals << 0.0f << 1.0f << 0.0f;

    for (quint32 i = 0; i < static_cast<quint32>(n - 1); i++) {
        quint32 l0 = i;
        quint32 l1 = i + 1;
        quint32 r0 = static_cast<quint32>(n) + i;
        quint32 r1 = static_cast<quint32>(n) + i + 1;
        indices << l0 << l1 << r0;
        indices << l1 << r1 << r0;
    }
 
    const float TILE_V = 20.0f;

    QVector<float> arcLen(static_cast<int>(n), 0.0f);
    for (size_t i = 1; i < n; i++) {
        QVector2D mid_prev = (left[i - 1] + right[i - 1]) * 0.5f;
        QVector2D mid_curr = (left[i] + right[i]) * 0.5f;
        arcLen[static_cast<int>(i)] =
            arcLen[static_cast<int>(i - 1)] + (mid_curr - mid_prev).length();
    }

    QVector<float> uvs;
    uvs.reserve(static_cast<int>(n) * 2 * 2);
    for (size_t i = 0; i < n; i++) uvs << 0.0f << (arcLen[static_cast<int>(i)] / TILE_V);
    for (size_t i = 0; i < n; i++) uvs << 1.0f << (arcLen[static_cast<int>(i)] / TILE_V);

    m_trackEntity = new Qt3DCore::QEntity(m_sceneRoot);
    Qt3DRender::QGeometryRenderer* renderer = new Qt3DRender::QGeometryRenderer(m_trackEntity);
    Qt3DCore::QGeometry* geometry = new Qt3DCore::QGeometry(renderer);

    // Position
    Qt3DCore::QBuffer* vertexBuffer = new Qt3DCore::QBuffer(geometry);
    vertexBuffer->setData(QByteArray(reinterpret_cast<const char*>(vertices.constData()),
        vertices.size() * sizeof(float)));
    Qt3DCore::QAttribute* posAttr = new Qt3DCore::QAttribute(geometry);
    posAttr->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
    posAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
    posAttr->setVertexSize(3);
    posAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    posAttr->setBuffer(vertexBuffer);
    posAttr->setByteStride(3 * sizeof(float));
    posAttr->setCount(static_cast<uint>(n * 2));
    geometry->addAttribute(posAttr);

    // Normals
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

    // UVs
    Qt3DCore::QBuffer* uvBuffer = new Qt3DCore::QBuffer(geometry);
    uvBuffer->setData(QByteArray(reinterpret_cast<const char*>(uvs.constData()),
        uvs.size() * sizeof(float)));
    Qt3DCore::QAttribute* uvAttr = new Qt3DCore::QAttribute(geometry);
    uvAttr->setName(Qt3DCore::QAttribute::defaultTextureCoordinateAttributeName());
    uvAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
    uvAttr->setVertexSize(2);
    uvAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    uvAttr->setBuffer(uvBuffer);
    uvAttr->setByteStride(2 * sizeof(float));
    uvAttr->setCount(static_cast<uint>(n * 2));
    geometry->addAttribute(uvAttr);

    // Indices
    Qt3DCore::QBuffer* indexBuffer = new Qt3DCore::QBuffer(geometry);
    indexBuffer->setData(QByteArray(reinterpret_cast<const char*>(indices.constData()),
        indices.size() * sizeof(quint32)));
    Qt3DCore::QAttribute* indexAttr = new Qt3DCore::QAttribute(geometry);
    indexAttr->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
    indexAttr->setVertexBaseType(Qt3DCore::QAttribute::UnsignedInt);
    indexAttr->setBuffer(indexBuffer);
    indexAttr->setCount(static_cast<uint>(indices.size()));
    geometry->addAttribute(indexAttr);

    renderer->setGeometry(geometry);
    renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    m_trackEntity->addComponent(renderer);

    // ── Matériau texturé 
    Qt3DExtras::QDiffuseMapMaterial* material =
        new Qt3DExtras::QDiffuseMapMaterial(m_trackEntity);

    Qt3DRender::QTextureImage* trackTex = new Qt3DRender::QTextureImage();
    trackTex->setSource(QUrl::fromLocalFile(
        QDir::currentPath() + track->getCurrentChoixMapData().trackData.trackTexturePath));

    material->diffuse()->addTextureImage(trackTex);
    material->setAmbient(track->getCurrentChoixMapData().trackData.ambientColor); 
    material->setSpecular(QColor(0, 0, 0));
    material->setShininess(0.0f);
    material->setTextureScale(1.0f);  

    m_trackEntity->addComponent(material);

    auto buildKerb = [&](const std::vector<QVector2D>& edge, const std::vector<QVector2D>& center, bool flipWinding) {
		float kerbWidth = track->getCurrentChoixMapData().trackData.kerbData.width;
		float kerbHeight = track->getCurrentChoixMapData().trackData.kerbData.height;
        float segmentLength = 10.0f;
        float accumulated = 0.0f;
        bool isRed = true;

        if(flipWinding) {
			kerbHeight = 0.15f;
		}

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

        auto makeEntity = [&](QVector<float>& verts, QVector<float>& normals,
            QVector<quint32>& idx, QColor color) {
                if (verts.isEmpty()) return;

                Qt3DCore::QEntity* entity = new Qt3DCore::QEntity(m_sceneRoot);
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

        makeEntity(redVerts, redNormals, redIdx, track->getCurrentChoixMapData().trackData.kerbData.color1);
        makeEntity(whiteVerts, whiteNormals, whiteIdx, track->getCurrentChoixMapData().trackData.kerbData.color2);
        };

    
		buildKerb(track->getTrackEdges().left, track->getCenterLine(), true); 
		buildKerb(track->getTrackEdges().right, track->getCenterLine(), false); 

    auto buildPitMesh = [&](const std::vector<QVector2D>& left,
        const std::vector<QVector2D>& right) {
            if (left.size() < 2 || right.size() < 2) {
               
                return;
            }
            if (left.size() != right.size()) {
                
                return;
            }
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
                quint32 l0 = i,      l1 = i + 1;
                quint32 r0 = static_cast<quint32>(pn) + i;
                quint32 r1 = static_cast<quint32>(pn) + i + 1;
                idx << l0 << l1 << r0;
                idx << l1 << r1 << r0;
            }

            const float TILE_V = 20.0f;
            QVector<float> arcLen(static_cast<int>(pn), 0.0f);
            for (size_t i = 1; i < pn; i++) {
                QVector2D mid_prev = (left[i - 1] + right[i - 1]) * 0.5f;
                QVector2D mid_curr = (left[i]     + right[i]    ) * 0.5f;
                arcLen[static_cast<int>(i)] =
                    arcLen[static_cast<int>(i - 1)] + (mid_curr - mid_prev).length();
            }
            QVector<float> uvs;
            uvs.reserve(static_cast<int>(pn) * 2 * 2);
            for (size_t i = 0; i < pn; i++) uvs << 0.0f << (arcLen[static_cast<int>(i)] / TILE_V);
            for (size_t i = 0; i < pn; i++) uvs << 1.0f << (arcLen[static_cast<int>(i)] / TILE_V);

            Qt3DCore::QEntity* pitEntity = new Qt3DCore::QEntity(m_sceneRoot);
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

            // UVs
            Qt3DCore::QBuffer* uvb = new Qt3DCore::QBuffer(pitGeom);
            uvb->setData(QByteArray(reinterpret_cast<const char*>(uvs.constData()),
                uvs.size() * sizeof(float)));
            Qt3DCore::QAttribute* uvAttr = new Qt3DCore::QAttribute(pitGeom);
            uvAttr->setName(Qt3DCore::QAttribute::defaultTextureCoordinateAttributeName());
            uvAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
            uvAttr->setVertexSize(2);
            uvAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
            uvAttr->setBuffer(uvb);
            uvAttr->setByteStride(2 * sizeof(float));
            uvAttr->setCount(static_cast<uint>(pn * 2));
            pitGeom->addAttribute(uvAttr);

            // Indices
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
            pitEntity->addComponent(pitRenderer);

            Qt3DExtras::QDiffuseMapMaterial* mat =
                new Qt3DExtras::QDiffuseMapMaterial(pitEntity);
            Qt3DRender::QTextureImage* pitTex = new Qt3DRender::QTextureImage();
            pitTex->setSource(QUrl::fromLocalFile(
                QDir::currentPath() + track->getCurrentChoixMapData().pitData.pitTexturePath));
            mat->diffuse()->addTextureImage(pitTex);
            mat->setAmbient(track->getCurrentChoixMapData().pitData.ambientColor);
            mat->setSpecular(QColor(0, 0, 0));
            mat->setShininess(0.0f);
            mat->setTextureScale(1.0f);
            pitEntity->addComponent(mat);
        };
       
    if (track->hasPitLane()) {
        PitLane pit = track->getPitLane();

        std::vector<QVector2D> fullLeft, fullRight;

        for (const auto& p : pit.entryCurveEdges.left)  fullLeft.push_back(p);
        for (const auto& p : pit.edges.left)             fullLeft.push_back(p);
        for (const auto& p : pit.exitCurveEdges.left)    fullLeft.push_back(p);

        for (const auto& p : pit.entryCurveEdges.right)  fullRight.push_back(p);
        for (const auto& p : pit.edges.right)             fullRight.push_back(p);
        for (const auto& p : pit.exitCurveEdges.right)    fullRight.push_back(p);

        if (!fullLeft.empty() && fullLeft.size() > pit.entryCurveEdges.left.size()) {
            fullLeft.erase(fullLeft.begin() + pit.entryCurveEdges.left.size());
            fullRight.erase(fullRight.begin() + pit.entryCurveEdges.right.size());
        }

        size_t exitJunction = pit.entryCurveEdges.left.size() - 1 + pit.edges.left.size();
        if (exitJunction < fullLeft.size()) {
            fullLeft.erase(fullLeft.begin() + exitJunction);
            fullRight.erase(fullRight.begin() + exitJunction);
        }

        buildPitMesh(fullLeft, fullRight);
        
        if (track->hasPitLane()) {
            PitLane pit = track->getPitLane();
            if (pit.centerLine.size() >= 2) {

                int mid = pit.centerLine.size() / 2;
                QVector2D center = pit.centerLine[mid];
                QVector2D dir = (pit.centerLine[mid + 1] - pit.centerLine[mid]).normalized();
                QVector2D normal(-dir.y(), dir.x());

                float halfLen = 20.0f;   
                float halfWid = 15.0f; 

                QVector2D c0 = center - dir * halfLen - normal * halfWid;
                QVector2D c1 = center + dir * halfLen - normal * halfWid;
                QVector2D c2 = center + dir * halfLen + normal * halfWid;
                QVector2D c3 = center - dir * halfLen + normal * halfWid;

                float y = 0.15f; 

                QVector<float> verts = {
                    c0.x(), y, c0.y(),
                    c1.x(), y, c1.y(),
                    c2.x(), y, c2.y(),
                    c3.x(), y, c3.y()
                };
                QVector<float> normals = {
                    0, 1, 0,  0, 1, 0,  0, 1, 0,  0, 1, 0
                };
                QVector<quint32> idx = { 0, 2, 1,  0, 3, 2 };


                Qt3DCore::QEntity* boxEntity = new Qt3DCore::QEntity(m_sceneRoot);
                Qt3DRender::QGeometryRenderer* renderer = new Qt3DRender::QGeometryRenderer(boxEntity);
                Qt3DCore::QGeometry* geom = new Qt3DCore::QGeometry(renderer);

                Qt3DCore::QBuffer* vb = new Qt3DCore::QBuffer(geom);
                vb->setData(QByteArray(reinterpret_cast<const char*>(verts.constData()),
                    verts.size() * sizeof(float)));

                Qt3DCore::QAttribute* posAttr = new Qt3DCore::QAttribute(geom);
                posAttr->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
                posAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
                posAttr->setVertexSize(3);
                posAttr->setByteOffset(0);
                posAttr->setBuffer(vb);
                posAttr->setByteStride(3 * sizeof(float));
                posAttr->setCount(4);
                geom->addAttribute(posAttr);

                Qt3DCore::QBuffer* nb = new Qt3DCore::QBuffer(geom);
                nb->setData(QByteArray(reinterpret_cast<const char*>(normals.constData()),
                    normals.size() * sizeof(float)));

                Qt3DCore::QAttribute* normAttr = new Qt3DCore::QAttribute(geom);
                normAttr->setName(Qt3DCore::QAttribute::defaultNormalAttributeName());
                normAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
                normAttr->setVertexSize(3);
                normAttr->setByteOffset(0);
                normAttr->setBuffer(nb);
                normAttr->setByteStride(3 * sizeof(float));
                normAttr->setCount(4);
                geom->addAttribute(normAttr);

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

                Qt3DExtras::QPhongMaterial* mat = new Qt3DExtras::QPhongMaterial(boxEntity);
                mat->setDiffuse(QColor(255, 255, 0));   // yellow like in 2D
                mat->setAmbient(QColor(100, 100, 0));
                mat->setShininess(0.0f);

                
                Qt3DRender::QCullFace* cullFace = new Qt3DRender::QCullFace();
                cullFace->setMode(Qt3DRender::QCullFace::NoCulling);

                Qt3DRender::QRenderPass* renderPass = new Qt3DRender::QRenderPass();
                renderPass->addRenderState(cullFace);

                Qt3DRender::QTechnique* technique = new Qt3DRender::QTechnique();
                technique->addRenderPass(renderPass);

                Qt3DRender::QEffect* effect = mat->effect();
                for (auto* tech : effect->techniques()) {
                    for (auto* pass : tech->renderPasses()) {
                        pass->addRenderState(cullFace);
                    }
                }

                boxEntity->addComponent(renderer);
                boxEntity->addComponent(mat);
            }
        }

    }
}

void Track3DViewer::buildGround(Track* track)
{

    if (m_groundEntity) {
        m_groundEntity->setParent(static_cast<Qt3DCore::QEntity*>(nullptr));
        delete m_groundEntity;
    }
    


    m_groundEntity = new Qt3DCore::QEntity(m_sceneRoot);

    Qt3DExtras::QPlaneMesh* planeMesh = new Qt3DExtras::QPlaneMesh();
    planeMesh->setWidth(track->getCurrentChoixMapData().groundData.width);
    planeMesh->setHeight(track->getCurrentChoixMapData().groundData.height); 
    planeMesh->setMeshResolution(QSize(2, 2));

    Qt3DCore::QTransform* groundTransform = new Qt3DCore::QTransform(m_groundEntity);
	groundTransform->setTranslation(QVector3D(0, -1.0f, 0)); 

    Qt3DExtras::QDiffuseMapMaterial* grassMat = new Qt3DExtras::QDiffuseMapMaterial(m_groundEntity);

    Qt3DRender::QTextureImage* grassTex = new Qt3DRender::QTextureImage();
    grassTex->setSource(QUrl::fromLocalFile(
        QDir::currentPath() + track->getCurrentChoixMapData().groundData.texturePath));
    grassMat->diffuse()->addTextureImage(grassTex);
    grassMat->setAmbient(track->getCurrentChoixMapData().groundData.ambientColor);  
    grassMat->setSpecular(QColor(0, 0, 0));
    grassMat->setShininess(0.0f);
    grassMat->setTextureScale(20.0f);

    m_groundEntity->addComponent(planeMesh);
    m_groundEntity->addComponent(grassMat);
    m_groundEntity->addComponent(groundTransform);
}

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
        
        Qt3DCore::QEntity* cpEntity = new Qt3DCore::QEntity(m_sceneRoot);

        QVector2D center2D = (cp.left + cp.right) / 2.0f;
      
        QVector3D fwdAxis(cp.forward.x(), 0.0f, cp.forward.y());
        QVector3D upAxis(0.0f, 1.0f, 0.0f);
        QVector3D rightAxis = QVector3D::crossProduct(upAxis, fwdAxis).normalized();

        Qt3DCore::QTransform* transform = new Qt3DCore::QTransform(cpEntity);

        transform->setTranslation(QVector3D(center2D.x(), 0.0f, center2D.y()));
		transform->setRotation(Qt3DCore::QTransform::fromAxes(rightAxis, upAxis, fwdAxis));
        float scale = 40.0f; 
		transform->setScale(40.0f);
        cpEntity->addComponent(transform);

        Qt3DCore::QEntity* offsetEntity = new Qt3DCore::QEntity(cpEntity);
        Qt3DCore::QTransform* offsetTransform = new Qt3DCore::QTransform(offsetEntity);

        if (i == 0) {
            offsetTransform->setTranslation(QVector3D(-1.0f / 2, 0.0f, 0.0f));
        }

        else {
            offsetTransform->setTranslation(QVector3D(-1.87f / 2, 0.0f, -0.33f / 2));
        }
      
        offsetEntity->addComponent(offsetTransform);

        Qt3DCore::QEntity* modelEntity = new Qt3DCore::QEntity(offsetEntity);
        
        Qt3DRender::QSceneLoader* loader = new Qt3DRender::QSceneLoader(modelEntity);

        connect(loader, &Qt3DRender::QSceneLoader::statusChanged,
            [modelEntity, i](Qt3DRender::QSceneLoader::Status status) {

                if (status == Qt3DRender::QSceneLoader::Error) {

                    return;
                }
                if (status != Qt3DRender::QSceneLoader::Ready) return;

                QList<Qt3DExtras::QPhongMaterial*> mats =
                    modelEntity->findChildren<Qt3DExtras::QPhongMaterial*>();

                for (Qt3DExtras::QPhongMaterial* mat : mats) {
                    mat->setShininess(0.0f);
                    mat->setSpecular(QColor(0, 0, 0));
                    QColor diff = mat->diffuse();
                    mat->setAmbient(diff.darker(200));
                }
            });

        modelEntity->addComponent(loader);

        QString modelPath;

        if (i == 0) {
            modelPath = QDir::currentPath() + "/3dModels/dae/overheadRound.dae"; 
        }
        else {
            modelPath = QDir::currentPath() + "/3dModels/dae/overheadRoundColored.dae"; 
        }

        loader->setSource(QUrl::fromLocalFile(modelPath));

        m_checkpointEntities.push_back(cpEntity);
    }
}

void Track3DViewer::buildCar()
{

    m_carEntity = new Qt3DCore::QEntity(m_sceneRoot);
    m_carTransform = new Qt3DCore::QTransform(m_carEntity);
    m_carTransform->setTranslation(QVector3D(0, 5.0f, 0));
    m_carEntity->addComponent(m_carTransform);

  
    Qt3DCore::QEntity* modelEntity = new Qt3DCore::QEntity(m_carEntity);

    Qt3DRender::QSceneLoader* loader = new Qt3DRender::QSceneLoader(modelEntity);
    loader->setSource(QUrl::fromLocalFile(
        QDir::currentPath() + "/3dModels/dae/raceCarGreen.dae"
    ));

    Qt3DCore::QTransform* modelTransform = new Qt3DCore::QTransform(modelEntity);
    modelTransform->setScale(5.0f);
    modelTransform->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, -90));

    modelEntity->addComponent(loader);
    modelEntity->addComponent(modelTransform);

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
}

void Track3DViewer::buildDecors(Track* track)
{
    m_loaderCache.clear();

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

        Qt3DCore::QEntity* decorEntity = new Qt3DCore::QEntity(m_sceneRoot);

        Qt3DCore::QTransform* decorTransform = new Qt3DCore::QTransform(decorEntity);
        float x = decor->getInfo().pos.x();
        float y = decor->getInfo().pos.y();
        float angle = decor->getInfo().angle;

        decorTransform->setTranslation(QVector3D(x, 0.0f, y));
        decorTransform->setRotation(
            QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0),
                -qRadiansToDegrees(angle)));
        decorEntity->addComponent(decorTransform);

        Qt3DCore::QEntity* modelEntity = new Qt3DCore::QEntity(decorEntity);

        QDir sceneDir(QCoreApplication::applicationDirPath() + "/sceneparsers");

        QString modelPath = QDir::currentPath() + decor->getInfo().modelPath;
        Qt3DRender::QSceneLoader* loader;

        if (m_loaderCache.contains(modelPath)) {

            loader = new Qt3DRender::QSceneLoader(modelEntity);
            loader->setSource(m_loaderCache[modelPath]->source());
        }
        else {
            loader = new Qt3DRender::QSceneLoader(modelEntity);
            loader->setSource(QUrl::fromLocalFile(modelPath));
            m_loaderCache[modelPath] = loader;
        }

        Qt3DCore::QTransform* modelTransform = new Qt3DCore::QTransform(modelEntity);
        modelTransform->setScale3D(QVector3D(
            decor->getInfo().width * decor->getScale(),  
            decor->getInfo().height * decor->getScale(),
            decor->getInfo().length * decor->getScale()));
        modelEntity->addComponent(loader);
        modelEntity->addComponent(modelTransform);

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

}

void Track3DViewer::buildBezierWalls(Track* track)
{

    for (Qt3DCore::QEntity* e : m_wallEntities) {
        e->setParent(static_cast<Qt3DCore::QEntity*>(nullptr));
        delete e;
    }
    m_wallEntities.clear();

    if (!track || !track->hasBezierCurves()) return;

    const auto& curves = track->getBezierCurves();


    auto evalBezier = [](const BezierCurveData& c, float t) -> QVector3D {
        float u = 1.0f - t;
        QVector2D p = c.p0 * (u * u * u)
            + c.p1 * (3 * u * u * t)
            + c.p2 * (3 * u * t * t)
            + c.p3 * (t * t * t);
        return QVector3D(p.x(), 0.0f, p.y());
        };
	float scale = 20.0f; 
    int   segmentsPerCurve = 20;  
    float wallModelLength = 0.12 * scale; 

    for (const BezierCurveData& curve : curves) {
        for (int i = 0; i < segmentsPerCurve; i++) {
            float t0 = (float)i / segmentsPerCurve;
            float t1 = (float)(i + 1) / segmentsPerCurve;

            QVector3D p0 = evalBezier(curve, t0);
            QVector3D p1 = evalBezier(curve, t1);

            QVector3D dir = (p1 - p0);
            float     segLen = dir.length();
            QVector3D dirN = dir.normalized();

            QVector3D center = (p0 + p1) / 2.0f;

            Qt3DCore::QEntity* wallEntity = new Qt3DCore::QEntity(m_sceneRoot);

            Qt3DCore::QTransform* wallTransform = new Qt3DCore::QTransform(wallEntity);

            wallTransform->setTranslation(center);

            QQuaternion rot = QQuaternion::rotationTo(
                QVector3D(1, 0, 0),
                dirN              
            );
            wallTransform->setRotation(rot);

            wallTransform->setScale3D(QVector3D(
                segLen / wallModelLength * scale,
                1.0f * scale,
                1.0f * scale 
            ));

            wallEntity->addComponent(wallTransform);

            Qt3DCore::QEntity* modelEntity = new Qt3DCore::QEntity(wallEntity);
            Qt3DRender::QSceneLoader* loader =
                new Qt3DRender::QSceneLoader(modelEntity);

            loader->setSource(QUrl::fromLocalFile(
                QDir::currentPath() + "/3dModels/dae/barrierWhite.dae"));

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

}

void Track3DViewer::buildInstancedDecors(Track* track)
{
 
    for (Qt3DCore::QEntity* e : m_instancedDecorEntities) {
        e->setParent(static_cast<Qt3DCore::QEntity*>(nullptr));
        delete e;
    }
    m_instancedDecorEntities.clear();

    if (!track || track->getDecors().empty()) return;

    QMap<QString, QVector<QMatrix4x4>> transformsByModel;

    for (DecorPieces* d : track->getDecors()) {
        if (!d) continue;

        QString modelPath =
            QDir::currentPath() + d->getInfo().modelPath;

        QMatrix4x4 mat;
        mat.translate(d->getInfo().pos.x(), 0.0f,
            d->getInfo().pos.y());
        mat.rotate(-qRadiansToDegrees(d->getInfo().angle),
            0, 1, 0);
        mat.scale(d->getInfo().width * d->getScale(),
            d->getInfo().height * d->getScale(),
            d->getInfo().length * d->getScale());

        transformsByModel[modelPath].append(mat);
    }

    for (auto it = transformsByModel.begin();
        it != transformsByModel.end(); ++it)
    {
        const QString& modelPath = it.key();
        const QVector<QMatrix4x4>& transforms = it.value();

        MeshDataList meshList =
            DaeLoader::loadByMaterial(modelPath);

        if (!meshList.valid) {

            continue;
        }

        Qt3DCore::QEntity* e = MeshInstance::buildFromList(
            meshList, transforms, m_sceneRoot);

        if (e) m_instancedDecorEntities.push_back(e);
    }

}

Qt3DCore::QEntity* Track3DViewer::createBox(Qt3DCore::QEntity* parent,
    QVector3D size,
    QVector3D position,
    QColor color)
{

    Qt3DCore::QEntity* entity = new Qt3DCore::QEntity(parent);

    Qt3DExtras::QCuboidMesh* mesh = new Qt3DExtras::QCuboidMesh();
    mesh->setXExtent(size.x());
    mesh->setYExtent(size.y());
    mesh->setZExtent(size.z());

    Qt3DCore::QTransform* transform = new Qt3DCore::QTransform(entity);
    transform->setTranslation(position);

    Qt3DExtras::QPhongMaterial* mat = new Qt3DExtras::QPhongMaterial(entity);
    mat->setDiffuse(color);
    mat->setAmbient(color.darker(150));

    entity->addComponent(mesh);
    entity->addComponent(transform);
    entity->addComponent(mat);

    return entity;
}

void Track3DViewer::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_V) changeCameraMode();
}

void Track3DViewer::onUpdateFrame()
{
    if (m_vehicule)
        updateVehicule(m_vehicule);
}
#pragma once
#include "MeshConverter.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QMap>
#include <QDebug>

// ─────────────────────────────────────────────────────────────
// DaeLoader
// Parses a Collada (.dae) file using Qt's built-in XML reader.
// Returns one MeshData per <triangles> block (i.e. per material).
// No external dependencies required.
// ─────────────────────────────────────────────────────────────
class DaeLoader
{
public:
    // Returns all submeshes, one per material
    static MeshDataList loadByMaterial(const QString& path)
    {
        MeshDataList result;

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "DaeLoader: failed to open" << path;
            return result;
        }

        QXmlStreamReader xml(&file);

        // ── Float sources keyed by source id ─────────────────
        QMap<QString, QVector<float>> floatSources;
        QString currentSourceId;

        // ── vertices element: maps vertices id → pos source id
        QMap<QString, QString> verticesMap;
        QString activeVerticesId;

        // ── Resolved geometry arrays ──────────────────────────
        QVector<float> positions, normals, uvs;

        // ── Material effects: effect id → diffuse color ───────
        // e.g. "ID3" → QColor(grey)
        QMap<QString, QColor>  effectDiffuse;
        QMap<QString, QColor>  effectAmbient;
        QMap<QString, QColor>  effectSpecular;
        QMap<QString, float>   effectShininess;
        QString currentEffectId;
        QString currentColorTag;

        // ── Material → effect mapping ─────────────────────────
        // e.g. "mat_ID4" → "ID3"
        QMap<QString, QString> materialToEffect;
        QString currentMaterialId;

        // ── Per-triangles state ───────────────────────────────
        struct TriBlock {
            QString      materialId;
            QVector<quint32> posIndices;
            QVector<quint32> normIndices;
            QVector<quint32> uvIndices;
            int stride = 0;
            int posOffset = -1;
            int normOffset = -1;
            int uvOffset = -1;
        };
        QVector<TriBlock> triBlocks;
        TriBlock* currentBlock = nullptr;
        bool inTriangles = false;
        bool inEffect = false;
        bool inMaterial = false;

        while (!xml.atEnd() && !xml.hasError()) {
            xml.readNext();

            if (xml.isStartElement()) {
                const QString tag = xml.name().toString();

                // ── Source tracking ───────────────────────────
                if (tag == "source")
                    currentSourceId =
                    xml.attributes().value("id").toString();

                // ── Float array ───────────────────────────────
                if (tag == "float_array") {
                    QString id =
                        xml.attributes().value("id").toString();
                    QStringList parts =
                        xml.readElementText()
                        .split(' ', Qt::SkipEmptyParts);
                    QVector<float> floats;
                    floats.reserve(parts.size());
                    for (const QString& p : parts)
                        floats.append(p.toFloat());
                    floatSources[id] = floats;
                    floatSources[currentSourceId] = floats;
                }

                // ── Vertices element ──────────────────────────
                if (tag == "vertices")
                    activeVerticesId =
                    xml.attributes().value("id").toString();

                // ── Input inside vertices (not triangles) ─────
                if (tag == "input" && !inTriangles) {
                    QString semantic =
                        xml.attributes().value("semantic")
                        .toString();
                    QString src = xml.attributes()
                        .value("source").toString().remove('#');

                    if (semantic == "POSITION" &&
                        !activeVerticesId.isEmpty()) {
                        verticesMap[activeVerticesId] = src;
                        if (floatSources.contains(src))
                            positions = floatSources[src];
                    }
                    else if (semantic == "NORMAL" &&
                        normals.isEmpty() &&
                        floatSources.contains(src))
                        normals = floatSources[src];
                    else if (semantic == "TEXCOORD" &&
                        uvs.isEmpty() &&
                        floatSources.contains(src))
                        uvs = floatSources[src];
                }

                // ── Effect tracking ───────────────────────────
                if (tag == "effect") {
                    currentEffectId =
                        xml.attributes().value("id").toString();
                    inEffect = true;
                }

                // ── Material tracking ─────────────────────────
                if (tag == "material") {
                    currentMaterialId =
                        xml.attributes().value("id").toString();
                    inMaterial = true;
                }

                // <instance_effect url="#ID3"/>
                if (tag == "instance_effect" && inMaterial) {
                    QString effectRef = xml.attributes()
                        .value("url").toString().remove('#');
                    materialToEffect[currentMaterialId] = effectRef;
                }

                // ── Color blocks inside effect ────────────────
                if (inEffect && (tag == "diffuse" ||
                    tag == "ambient" ||
                    tag == "specular"))
                    currentColorTag = tag;

                if (inEffect && tag == "color" &&
                    !currentColorTag.isEmpty())
                {
                    QStringList parts =
                        xml.readElementText()
                        .split(' ', Qt::SkipEmptyParts);
                    if (parts.size() >= 3) {
                        QColor col = QColor::fromRgbF(
                            qBound(0.0f, parts[0].toFloat(), 1.0f),
                            qBound(0.0f, parts[1].toFloat(), 1.0f),
                            qBound(0.0f, parts[2].toFloat(), 1.0f),
                            parts.size() >= 4
                            ? qBound(0.0f,
                                parts[3].toFloat(), 1.0f)
                            : 1.0f);
                        if (currentColorTag == "diffuse")
                            effectDiffuse[currentEffectId] = col;
                        else if (currentColorTag == "ambient")
                            effectAmbient[currentEffectId] = col;
                        else if (currentColorTag == "specular")
                            effectSpecular[currentEffectId] = col;
                    }
                }

                if (inEffect && tag == "float" &&
                    xml.attributes().isEmpty()) {
                    float val = xml.readElementText().toFloat();
                    if (!effectShininess.contains(currentEffectId))
                        effectShininess[currentEffectId] = val;
                }

                // ── Triangles block ───────────────────────────
                if (tag == "triangles" || tag == "polylist") {
                    inTriangles = true;
                    triBlocks.append(TriBlock());
                    currentBlock = &triBlocks.last();
                    currentBlock->materialId =
                        xml.attributes().value("material")
                        .toString();
                }

                // ── Input inside triangles ────────────────────
                if (tag == "input" && inTriangles &&
                    currentBlock)
                {
                    QString semantic =
                        xml.attributes().value("semantic")
                        .toString();
                    QString src = xml.attributes()
                        .value("source").toString().remove('#');
                    int offset =
                        xml.attributes().value("offset").toInt();
                    currentBlock->stride =
                        qMax(currentBlock->stride, offset + 1);

                    if (semantic == "VERTEX") {
                        currentBlock->posOffset = offset;
                        QString realSrc =
                            verticesMap.contains(src)
                            ? verticesMap[src] : src;
                        if (positions.isEmpty() &&
                            floatSources.contains(realSrc))
                            positions = floatSources[realSrc];
                    }
                    else if (semantic == "NORMAL") {
                        currentBlock->normOffset = offset;
                        if (normals.isEmpty() &&
                            floatSources.contains(src))
                            normals = floatSources[src];
                    }
                    else if (semantic == "TEXCOORD") {
                        currentBlock->uvOffset = offset;
                        if (uvs.isEmpty() &&
                            floatSources.contains(src))
                            uvs = floatSources[src];
                    }
                }

                // ── Face indices ──────────────────────────────
                if (tag == "p" && inTriangles && currentBlock) {
                    QStringList parts =
                        xml.readElementText()
                        .split(' ', Qt::SkipEmptyParts);
                    int s = qMax(currentBlock->stride, 1);
                    for (int i = 0;
                        i + s - 1 < parts.size();
                        i += s)
                    {
                        if (currentBlock->posOffset >= 0)
                            currentBlock->posIndices.append(
                                parts[i + currentBlock->posOffset]
                                .toUInt());
                        if (currentBlock->normOffset >= 0)
                            currentBlock->normIndices.append(
                                parts[i + currentBlock->normOffset]
                                .toUInt());
                        if (currentBlock->uvOffset >= 0)
                            currentBlock->uvIndices.append(
                                parts[i + currentBlock->uvOffset]
                                .toUInt());
                    }
                }
            }

            if (xml.isEndElement()) {
                const QString tag = xml.name().toString();
                if (tag == "triangles" || tag == "polylist") {
                    inTriangles = false;
                    currentBlock = nullptr;
                }
                if (tag == "effect") { inEffect = false; }
                if (tag == "material") { inMaterial = false; }
                if (tag == "diffuse" ||
                    tag == "ambient" ||
                    tag == "specular")
                    currentColorTag.clear();
            }
        }

        file.close();

        if (positions.isEmpty() || triBlocks.isEmpty()) {
            qDebug() << "DaeLoader: no geometry in" << path;
            return result;
        }

        // ── Build one MeshData per triangles block ────────────
        for (const TriBlock& block : triBlocks) {
            if (block.posIndices.isEmpty()) continue;

            MeshData mesh;

            // Resolve material → effect → color
            // The material symbol in <triangles> matches via
            // <instance_material symbol="..." target="#mat_ID4"/>
            // but for simplicity we match by material id directly
            QString effectId;
            // Try direct match first
            if (materialToEffect.contains(block.materialId))
                effectId = materialToEffect[block.materialId];
            else {
                // Try finding a material whose id contains the symbol
                for (auto it = materialToEffect.begin();
                    it != materialToEffect.end(); ++it) {
                    if (it.key().contains(block.materialId) ||
                        block.materialId.contains(it.key())) {
                        effectId = it.value();
                        break;
                    }
                }
            }

            // Apply colors from effect
            if (!effectId.isEmpty()) {
                if (effectDiffuse.contains(effectId))
                    mesh.diffuse = effectDiffuse[effectId];
                if (effectAmbient.contains(effectId))
                    mesh.ambient = effectAmbient[effectId];
                else
                    mesh.ambient = QColor(
                        mesh.diffuse.red() * 0.8f,
                        mesh.diffuse.green() * 0.8f,
                        mesh.diffuse.blue() * 0.8f);
                if (effectSpecular.contains(effectId))
                    mesh.specular = effectSpecular[effectId];
                if (effectShininess.contains(effectId))
                    mesh.shininess = effectShininess[effectId];
            }
            else {
                // Fallback: use first available effect color
                if (!effectDiffuse.isEmpty())
                    mesh.diffuse = effectDiffuse.first();
                mesh.ambient = QColor(
                    mesh.diffuse.red() * 0.8f,
                    mesh.diffuse.green() * 0.8f,
                    mesh.diffuse.blue() * 0.8f);
            }

            // Build flat vertex list
            mesh.vertices.reserve(block.posIndices.size());
            mesh.indices.reserve(block.posIndices.size());

            for (int i = 0; i < block.posIndices.size(); i++) {
                MeshVertex vert;

                quint32 pi = block.posIndices[i];
                if ((int)(pi * 3 + 2) < positions.size())
                    vert.position = QVector3D(
                        positions[pi * 3 + 0],
                        positions[pi * 3 + 1],
                        positions[pi * 3 + 2]);

                if (!block.normIndices.isEmpty()) {
                    quint32 ni = block.normIndices[i];
                    if ((int)(ni * 3 + 2) < normals.size())
                        vert.normal = QVector3D(
                            normals[ni * 3 + 0],
                            normals[ni * 3 + 1],
                            normals[ni * 3 + 2]);
                }

                if (!block.uvIndices.isEmpty()) {
                    quint32 ti = block.uvIndices[i];
                    if ((int)(ti * 2 + 1) < uvs.size())
                        vert.uv = QVector2D(
                            uvs[ti * 2 + 0],
                            uvs[ti * 2 + 1]);
                }

                mesh.indices.append(static_cast<quint32>(i));
                mesh.vertices.append(vert);
            }

            mesh.valid = true;
            result.meshes.append(mesh);
        }

        result.valid = !result.meshes.isEmpty();

        qDebug() << "DaeLoader: loaded" << path
            << "-" << result.meshes.size() << "submeshes";

        return result;
    }

    // Convenience: load as single merged mesh (original behavior)
    static MeshData load(const QString& path)
    {
        MeshDataList list = loadByMaterial(path);
        if (!list.valid || list.meshes.isEmpty()) return MeshData();

        // If only one submesh return it directly
        if (list.meshes.size() == 1) return list.meshes.first();

        // Otherwise merge all submeshes into one
        // (loses per-material colors but keeps geometry)
        MeshData merged;
        for (const MeshData& m : list.meshes) {
            quint32 offset = static_cast<quint32>(merged.vertices.size());
            for (const MeshVertex& v : m.vertices)
                merged.vertices.append(v);
            for (quint32 idx : m.indices)
                merged.indices.append(idx + offset);
        }
        merged.diffuse = list.meshes.first().diffuse;
        merged.ambient = list.meshes.first().ambient;
        merged.specular = list.meshes.first().specular;
        merged.valid = true;
        return merged;
    }
};
#include "PitStop.h"
#include <algorithm>
#include <QDebug>

PitStop::PitStop()
{
    zone = QRect(300, 250, 80, 80);
    leavingPitStop = false;
}

PitStop::~PitStop()
{
}

bool PitStop::contains(int carX, int carY)
{
    return zone.contains(carX, carY);
}

void PitStop::recharge(float deltaTime, float& carburant, float& nos)
{
    carburant = std::min(carburant + rechargeRate * deltaTime, 100.0f);
    nos = std::min(nos + rechargeRate * deltaTime, 100.0f);

    qDebug() << "[PIT STOP] Recharge en cours | Carburant:" << (int)carburant << "% | NOS:" << (int)nos << "%";
}

void PitStop::placeNearTrack(const std::vector<QVector2D>& centerLine, float scale, float offset, float trackWidth)
{
    if (centerLine.size() < 2) return;

    pitLaneScale = scale;

    int index = centerLine.size() / 4;
    QVector2D point = centerLine[index];

    QVector2D dir = (centerLine[index + 1] - centerLine[index]).normalized();
    QVector2D normal(-dir.y(), dir.x());

    pitLaneDir = dir;
    pitLaneNormal = normal;

    float laneHalfLen = 50.0f;   // demi-longueur du trapèze
    float laneDepth = 35.0f;   // profondeur vers la droite
    float laneTopHalf = 20.0f;   // demi-largeur du côté court (côté piste)

    // Bord droit de la piste
    QVector2D trackRightEdge = point + normal * (trackWidth / 2.0f);

    // 4 coins du trapèze en coords monde
    // Côté piste (grand côté maintenant)
    trapeze[0] = trackRightEdge - dir * laneHalfLen;   // haut-gauche
    trapeze[1] = trackRightEdge + dir * laneHalfLen;   // bas-gauche
    // Côté extérieur (petit côté maintenant)
    trapeze[2] = trackRightEdge + normal * laneDepth + dir * laneTopHalf;   // bas-droite
    trapeze[3] = trackRightEdge + normal * laneDepth - dir * laneTopHalf;   // haut-droite

    // Points d'entrée/sortie
    pitEntry = trapeze[0];
    pitExit = trapeze[1];

    // Pit stop au centre du trapèze, un peu plus grand
    QVector2D center = (trapeze[0] + trapeze[1] + trapeze[2] + trapeze[3]) / 4.0f;
    int px = (int)(center.x() * scale) - 35;
    int py = (int)(center.y() * scale) - 25;
    zone = QRect(px - 45, py - 30, 90, 60);  // encore plus grand
}

QPainterPath PitStop::getPitLanePath(float scale) const
{
    QPainterPath path;
    path.moveTo(trapeze[0].x() * scale, trapeze[0].y() * scale);
    path.lineTo(trapeze[1].x() * scale, trapeze[1].y() * scale);
    path.lineTo(trapeze[2].x() * scale, trapeze[2].y() * scale);
    path.lineTo(trapeze[3].x() * scale, trapeze[3].y() * scale);
    path.closeSubpath();
    return path;
}
#pragma once
#include <QRect>
#include <QVector2D>
#include <vector>
#include <QPolygonF>
#include <QPainterPath>

class PitStop
{
public:
    PitStop();
    PitStop(int x, int y, int width, int height);
    ~PitStop();

    bool contains(int carX, int carY);
    void recharge(float deltaTime, float& carburant, float& nos);
    void placeNearTrack(const std::vector<QVector2D>& centerLine, float scale, float offset = 150.0f, float trackWidth = 40.0f);

    QRect getRect() const { return zone; }
    bool isLeaving() const { return leavingPitStop; }
    void setLeaving(bool b) { leavingPitStop = b; }
    void resetLeaving() { leavingPitStop = false; }
    QPainterPath getPitLanePath(float scale) const;
    QVector2D getPitEntry() const { return pitEntry; }
    QVector2D getPitExit()  const { return pitExit; }

private:
    QRect zone;
    float rechargeRate = 20.0f;
    bool leavingPitStop = false;
    QVector2D pitEntry;
    QVector2D pitExit;
    QVector2D pitCenter;
    QVector2D pitLaneDir;
    QVector2D pitLaneNormal;
    QVector2D trapeze[4];
    float pitLaneScale = 5.0f;
};
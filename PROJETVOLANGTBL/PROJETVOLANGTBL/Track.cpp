#include "Track.h"
Track::Track()
{
    piecesIntList = { 6, 5, 3, 5, 5, 5, 3, 5, 5, 5, 3, 5, 5, 5, 3, 5 };
    trackWidth = 20.0f;
	startAngle = 0.0f;
    currentAngle = startAngle;
    currentPos = Vec2(0, 0);
}

Track::Track(std::vector<int> listPieces)
{
	for (size_t i = 0; i < listPieces.size(); i++) {
        piecesIntList.push_back(listPieces[i]);
    }
    trackWidth = 20.0f;
    startAngle = 0.0f;
    currentAngle = startAngle;
	currentPos = Vec2(0, 0);
}



void Track::calculateTrackEdges()
{

    float halfWidth = trackWidth * 0.5f;

    for (size_t i = 0; i < centerLine.size(); i++) {

        Vec2 dir;

        if (i == 0) {
            // first point → forward direction
            dir = (centerLine[1] - centerLine[0]).normalized();
        }
        else if (i == centerLine.size() - 1) {
            // last point → backward direction
            dir = (centerLine[i] - centerLine[i - 1]).normalized();
        }
        else {
            // middle point → average direction (smooth turns)
            Vec2 dir1 = (centerLine[i] - centerLine[i - 1]).normalized();
            Vec2 dir2 = (centerLine[i + 1] - centerLine[i]).normalized();
            dir = (dir1 + dir2).normalized();
        }

        Vec2 normal = dir.perpendicular();

        trackEdges.left.push_back(centerLine[i] + normal * halfWidth);
        trackEdges.right.push_back(centerLine[i] - normal * halfWidth);
    }

}

void Track::calculAngLen(int index)
{
    std::vector<float> angles = pieces[index].getAngles();
	std::vector<float> lengths = pieces[index].getLengths();
    for (size_t i = 0; i < angles.size(); i++) {
        currentAngle += angles[i];
        currentPos = currentPos.move(currentAngle, lengths[i]);
        centerLine.push_back(currentPos);
    }
    calculateTrackEdges();
}



Virage_45right::Virage_45right() {

    id = 1;
    angles = { 0, 45.0f / 2, 0, 45.0f / 2, 0 };
    lengths = { 20, 10, 10, 10, 20 };
}
Virage_45left::Virage_45left() {
    id = 2;
    angles = { 0, -45.0f / 2, 0, -45.0f / 2, 0 };
    lengths = { 20, 10, 10, 10, 20 };
}
Virage_90right::Virage_90right() {
    id = 3;
    angles = { 0, 45, 0, 45, 0 };
    lengths = { 20, 10, 10, 10, 20 };
}
Virage_90left::Virage_90left() {
    id = 4;
    angles = { 0, -45, 0, -45, 0 };
    lengths = { 20, 10, 10, 10, 20 };
}
Straight::Straight() {
    id = 5;
    angles = { 0 };
    lengths = { 60 };
}
StartLine::StartLine() {
    id = 6;
    angles = { 0 };
    lengths = { 60 };
}
Garage::Garage() {
    id = 7;
    angles = { 0, 0, 0, 0, 0 };
    lengths = { 20, 20, 20, 20, 20 };
}
Pit::Pit() {
    id = 8;
    angles = { 0 };
    lengths = { 120 };
}
Grandstand::Grandstand() {
    id = 9;
    angles = { 0 };
    lengths = { 20 };
}
Bridges::Bridges() {
    id = 10;
    angles = { 0 };
    lengths = { 60 };
}

TrackPieces::~TrackPieces()
{

}

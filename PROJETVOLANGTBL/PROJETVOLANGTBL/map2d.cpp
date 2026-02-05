#include "map2d.h"


TrackEdges calculateTrackEdges(
    const std::vector<Vec2>& centerLine,
    float trackWidth)
{
    TrackEdges edges;

    if (centerLine.size() < 2)
        return edges;

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

        edges.left.push_back(centerLine[i] + normal * halfWidth);
        edges.right.push_back(centerLine[i] - normal * halfWidth);
    }

    return edges;
}



Map2D::Map2D()
{
    trackList.resize(1);
    Vec2 start(100, 100);
    trackList[0].start = start;
    float startAngle = 0.0;
    trackList[0].startAngle = startAngle; 
    
    // Define the "angles" for each segment
    std::vector<float> angles = { 0, 45, 0, -30, -45, 0, 30, 0, -60, 0 };
    // Define the "lengths" of each segment
    std::vector<float> lengths = { 50, 50, 50, 50, 50, 50, 50, 50, 50, 50 };

    std::vector<Vec2> track;
    track.push_back(start);


    float currentAngle = startAngle;
    Vec2 currentPos = start;

    for (size_t i = 0; i < angles.size(); i++) {
        currentAngle += angles[i];
        currentPos = currentPos.move(currentAngle, lengths[i]);
        track.push_back(currentPos);
    }
    trackList[0].centerLine = track;
    trackList[0].trackEdges = calculateTrackEdges(track, 2);



    //trackList[0]->trackEdges = trackEdges;
}

Map2D::~Map2D()
{

}

void Map2D::setChoixMap(int x)
{
	choixMap = x;
}

int Map2D::getChoixMap()
{
	return choixMap;
}

Vec2 Map2D::getStartPos()
{
    return trackList[choixMap].start;
}

std::vector<trackInfo> Map2D::getTrackList()
{
    return trackList;
}

void Map2D::addStartPos(Vec2 pos)
{
    trackList[choixMap].start = pos;
}

void Map2D::addStartAng(float ang)
{
    trackList[choixMap].startAngle = ang;
}


/*
void Map2D::trackGenerator(Vec2 start, float startAng,
    const std::vector<float>& segmentAngles,
    const std::vector<float>& segmentLengths)
{
      segmentAngles 
      std::vector<Vec2> track;
      track.push_back(start);

      float currentAngle = startAngle;
      Vec2 currentPos = start;

      for (size_t i = 0; i < angles.size(); i++) {
          currentAngle += angles[i];
          currentPos = currentPos.move(currentAngle, lengths[i]);
          track.push_back(currentPos);
      }

      
}
*/

struct GridPoint {
    int x;
    int y;
};

GridPoint toGrid(const Vec2& v,
    float minX, float maxX,
    float minY, float maxY)
{
    GridPoint p;
    p.x = int((v.x - minX) / (maxX - minX) * (GRID_W - 1));
    p.y = int((v.y - minY) / (maxY - minY) * (GRID_H - 1));

    // inversion Y pour la console
    p.y = GRID_H - 1 - p.y;

    return p;
}

void computeBounds(const trackInfo& track,
    float& minX, float& maxX,
    float& minY, float& maxY)
{
    minX = minY = 1e9f;
    maxX = maxY = -1e9f;

    auto check = [&](const Vec2& v) {
        minX = std::min(minX, v.x);
        maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y);
        maxY = std::max(maxY, v.y);
        };

    for (auto& p : track.centerLine) check(p);
    for (auto& p : track.trackEdges.left)  check(p);
    for (auto& p : track.trackEdges.right) check(p);
}

void drawTrackASCII(const trackInfo& track)
{
    char grid[GRID_H][GRID_W];

    // 1. Effacer la grille
    for (int y = 0; y < GRID_H; y++)
        for (int x = 0; x < GRID_W; x++)
            grid[y][x] = ' ';

    // 2. Calculer limites
    float minX, maxX, minY, maxY;
    computeBounds(track, minX, maxX, minY, maxY);

    // marge
    float margin = 10.0f;
    minX -= margin; maxX += margin;
    minY -= margin; maxY += margin;

    // 3. Dessiner centre
    for (auto& p : track.centerLine) {
        GridPoint g = toGrid(p, minX, maxX, minY, maxY);
        if (g.x >= 0 && g.x < GRID_W && g.y >= 0 && g.y < GRID_H)
            grid[g.y][g.x] = '.';
    }

    // 4. Dessiner bords
    for (auto& p : track.trackEdges.left) {
        GridPoint g = toGrid(p, minX, maxX, minY, maxY);
        if (g.x >= 0 && g.x < GRID_W && g.y >= 0 && g.y < GRID_H)
            grid[g.y][g.x] = '#';
    }

    for (auto& p : track.trackEdges.right) {
        GridPoint g = toGrid(p, minX, maxX, minY, maxY);
        if (g.x >= 0 && g.x < GRID_W && g.y >= 0 && g.y < GRID_H)
            grid[g.y][g.x] = '#';
    }

    // 5. Dessiner départ
    GridPoint s = toGrid(track.start, minX, maxX, minY, maxY);
    grid[s.y][s.x] = 'S';

    // 6. Affichage console
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++)
            std::cout << grid[y][x];
        std::cout << "\n";
    }
}

void Map2D::drawMapASCII() const
{
    if (trackList.empty()) {
        std::cout << "No track\n";
        return;
    }

    drawTrackASCII(trackList[choixMap]);
}


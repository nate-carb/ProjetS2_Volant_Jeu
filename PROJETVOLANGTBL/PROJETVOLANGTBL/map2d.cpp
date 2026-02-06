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
    std::vector<float> angles = { 0, 45/4, 45 / 2, 45/4, 0};

    /*std::vector<float> angles = { 
            0, 0, -45, 0, -45,
            0, 0, 0, 0, -45, 
            0, -45, 0, 0, 0, 
            0, -45, 0, -45, 0, 
            0, 0, 0, -45, 0, 
            -45, 0, 0 
    };*/
    // Define the "lengths" of each segment
    std::vector<float> lengths = {  20, 10, 10, 10, 20 };
    
    /*std::vector<float> lengths = { 
            20, 20, 20, 20, 20,
            20, 20, 20, 20, 20,
            20, 20, 20, 20, 20,
            20, 20, 20, 20, 20,
            20, 20, 20, 20, 20,
            20, 20, 20 
    };*/

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
    trackList[0].trackEdges = calculateTrackEdges(track, 40);



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

void Map2D::calculAngLen(
    std::vector<Vec2>& track,
    float& currentAngle, Vec2& currentPos,
    std::vector<float>& angles, std::vector<float>& lengths
) {
	
    for (size_t i = 0; i < angles.size(); i++) {
        currentAngle += angles[i];
        currentPos = currentPos.move(currentAngle, lengths[i]);
        track.push_back(currentPos);
    }
    trackList[trackListTaille - 1].centerLine = track;
    trackList[trackListTaille - 1].trackEdges = calculateTrackEdges(track, 20);
}

void Map2D::trackGenerator()  
{
	int size = trackList.size();
	trackList.resize(size++);
	trackListTaille = trackList.size();

	Vec2 start(0,0);
    /*int startAng;
    
    std::cout << "Entrer l'angle de depart (0 - 90): ";
    std::cin >> startAng;*/
    std::string input;

    std::cout << "Entrer l'angle de depart (0 - 90): ";
    std::getline(std::cin, input);
    int startAng = std::stoi(input); 

    std::vector<Vec2> track;
    track.push_back(start);


    float currentAngle = startAng;
    Vec2 currentPos = start;

    std::vector<float> angles;
    std::vector<float> lengths;
    std::string trackIn;
    while (true) {

        std::cout << "Entrer type de track (45g, 45d, 90g, 90d, droit) END pour la fin: ";
        std::cin >> trackIn;
        

        if (trackIn == "END") {
            break;  // Exit the loop
        }

        if (trackIn == "45g") {
            // Define the "angles" for each segment
            angles = { 0, 45 / 2, 0, 45 / 2, 0 };
            // Define the "lengths" of each segment
            lengths = { 20, 10, 10, 10, 20 };
            calculAngLen(track, currentAngle, currentPos, angles, lengths);
            drawMapASCII();
        }
        else if (trackIn == "45d") {
            // Define the "angles" for each segment
            angles = { 0, -45 / 2, 0, -45 / 2, 0 };
            // Define the "lengths" of each segment
            lengths = { 20, 10, 10, 10, 20 };
            calculAngLen(track, currentAngle, currentPos, angles, lengths);
            drawMapASCII();
        }
        else if (trackIn == "90g") {
            // Define the "angles" for each segment
            angles = { 0, 45, 0, 45, 0 };
            // Define the "lengths" of each segment
            lengths = { 20, 10, 10, 10, 20 };
            calculAngLen(track, currentAngle, currentPos, angles, lengths);
            drawMapASCII();
        }
        else if (trackIn == "90d") {
            // Define the "angles" for each segment
            angles = { 0, -45, 0, -45, 0 };
            // Define the "lengths" of each segment
            lengths = { 20, 10, 10, 10, 20 };
            calculAngLen(track, currentAngle, currentPos, angles, lengths);
            drawMapASCII();
        }
        else if (trackIn == "droit") {
            // Define the "angles" for each segment
            angles = { 0, 0, 0, 0, 0 };
            // Define the "lengths" of each segment
            lengths = { 20, 20, 20, 20, 20 };
            calculAngLen(track, currentAngle, currentPos, angles, lengths);
            drawMapASCII();
        }
        else {
            std::cout << "Type de track invalide.\n";
        }

    }
}


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

void drawLine(char grid[GRID_H][GRID_W], GridPoint p0, GridPoint p1, char ch)
{
    // Bresenham's line algorithm
    int dx = abs(p1.x - p0.x);
    int dy = abs(p1.y - p0.y);
    int sx = (p0.x < p1.x) ? 1 : -1;
    int sy = (p0.y < p1.y) ? 1 : -1;
    int err = dx - dy;

    int x = p0.x;
    int y = p0.y;

    while (true) {
        // Draw point if in bounds
        if (x >= 0 && x < GRID_W && y >= 0 && y < GRID_H)
            grid[y][x] = ch;

        // Check if we've reached the end
        if (x == p1.x && y == p1.y)
            break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
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
            grid[g.y][g.x] = '-';
    }
    // 4. Draw left edge (connected)
    for (size_t i = 0; i < track.trackEdges.left.size(); i++) {
        GridPoint g = toGrid(track.trackEdges.left[i], minX, maxX, minY, maxY);

        if (i > 0) {
            GridPoint prev = toGrid(track.trackEdges.left[i - 1], minX, maxX, minY, maxY);
            drawLine(grid, prev, g, '#');
        }
    }

    // 5. Draw right edge (connected)
    for (size_t i = 0; i < track.trackEdges.right.size(); i++) {
        GridPoint g = toGrid(track.trackEdges.right[i], minX, maxX, minY, maxY);

        if (i > 0) {
            GridPoint prev = toGrid(track.trackEdges.right[i - 1], minX, maxX, minY, maxY);
            drawLine(grid, prev, g, '#');
        }
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


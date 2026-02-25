#include "map2d.h"

/*
Map2D::Map2D()
{

}

Map2D::~Map2D()
{

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

void Map2D::computeBounds(
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

    for (auto& p : getCenterLine()) check(p);
    for (auto& p : getTrackEdges().left)  check(p);
    for (auto& p : getTrackEdges().right) check(p);
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


void Map2D::drawTrackASCII()
{
    char grid[GRID_H][GRID_W];

    // 1. Effacer la grille
    for (int y = 0; y < GRID_H; y++)
        for (int x = 0; x < GRID_W; x++)
            grid[y][x] = ' ';

    // 2. Calculer limites
    float minX, maxX, minY, maxY;
    computeBounds(minX, maxX, minY, maxY);

    // marge
    float margin = 10.0f;
    minX -= margin; maxX += margin;
    minY -= margin; maxY += margin;

    // 3. Dessiner centre
    for (auto& p : getCenterLine()) {
        GridPoint g = toGrid(p, minX, maxX, minY, maxY);
        if (g.x >= 0 && g.x < GRID_W && g.y >= 0 && g.y < GRID_H)
            grid[g.y][g.x] = '-';
    }

    // 4. Draw left edge (connected)
    for (size_t i = 0; i < getTrackEdges().left.size(); i++) {
        GridPoint g = toGrid(getTrackEdges().left[i], minX, maxX, minY, maxY);

        if (i > 0) {
            GridPoint prev = toGrid(getTrackEdges().left[i - 1], minX, maxX, minY, maxY);
            drawLine(grid, prev, g, '#');
        }
    }

    // 5. Draw right edge (connected)
    for (size_t i = 0; i < getTrackEdges().right.size(); i++) {
        GridPoint g = toGrid(getTrackEdges().right[i], minX, maxX, minY, maxY);

        if (i > 0) {
            GridPoint prev = toGrid(getTrackEdges().right[i - 1], minX, maxX, minY, maxY);
            drawLine(grid, prev, g, '#');
        }
    }

    // 5. Dessiner départ
    Vec2 start(0, 0); // Assuming start is at (0,0) for this example
    GridPoint s = toGrid(start, minX, maxX, minY, maxY);
    grid[s.y][s.x] = 'S';

    // 6. Affichage console
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++)
            std::cout << grid[y][x];
        std::cout << "\n";
    }
}

void Map2D::displayTrack()
{
    drawTrackASCII();
}
*/
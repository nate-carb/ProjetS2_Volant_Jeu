#include "Track.h"
#include <fstream>
#include <sstream>

QVector2D perpendicular(QVector2D v)
{

    return QVector2D(-v.y(), v.x());
}
QVector2D move(QVector2D v, float angleDeg, float distance)
{
    float rad = angleDeg * (3.14159265f / 180.0f);
    return QVector2D(v.x() + distance * cos(rad), v.y() + distance * sin(rad));
}

float distancePointToSegment(const QVector2D& P,
    const QVector2D& A,
    const QVector2D& B)
{
    QVector2D AB = B - A; //Vector entre les points central
	QVector2D AP = P - A; //Vector entre le point et le point central

    float ab2 = QVector2D::dotProduct(AB, AB);

	// Si AB est un point, alors la distance est simplement la distance entre P et A
    if (ab2 == 0.0f)
        return (P - A).length();

	// Projection de AP sur AB, normalisée par la longueur de AB au carré
    float t = QVector2D::dotProduct(AP, AB) / ab2;

    t = std::max(0.0f, std::min(1.0f, t));

    QVector2D closestPoint = A + AB * t;

	// La distance entre P et le point le plus proche sur le segment AB
    return (P - closestPoint).length();
}
bool Track::isVector2DOnTrack(const QVector2D& point) const
{
    float minDist = 300000;//Valeur arbitraire très grande pour initialiser la distance minimale
    for (size_t i = 1; i < centerLine.size(); i++) {

		// Calcule la distance du point au segment formé par centerLine[i-1] et centerLine[i]
        float d = distancePointToSegment(point, centerLine[i - 1], centerLine[i]);
        minDist = std::min(minDist, d); // Met à jour la distance minimale si nécessaire
    }
    return minDist <= trackWidth / 2.0f; // Vérifie si la distance minimale est inférieure ou égale à la moitié de la largeur de la piste
}
// TrackPieces base class
TrackPieces::~TrackPieces() {}


// Virage_45right
Virage_45right::Virage_45right() {
    id = VIRAGE_45RIGHT;
    pos = 0;
    spritePath = "trackPieces/versionPNG/Virage45Right.png";
    spriteRotationOffset = 0.0f;
    //angles = { 0, 45.0f / 4, 45.0f / 4,  45.0f / 4, 45.0f / 4, 0};
    angles = { 45.0f / 4, 45.0f / 4,  45.0f / 4, 45.0f / 4 };
    //lengths = { 0, 10, 10, 10, 10, 0 };
    lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
}

// Virage_45left
Virage_45left::Virage_45left() {
    id = VIRAGE_45LEFT;
    pos = 0;
    spritePath = "trackPieces/versionPNG/Virage45Left.png";
    //angles = {0, -45.0f / 4, -45.0f / 4,  -45.0f / 4, -45.0f / 4, 0 };
    angles = { -45.0f / 4, -45.0f / 4,  -45.0f / 4, -45.0f / 4 };
    //lengths = {0, 10, 10, 10, 10, 0 };
    lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
}

// Virage_90right
Virage_90right::Virage_90right() {
    id = VIRAGE_90RIGHT;
    pos = 0;
    spritePath = "trackPieces/versionPNG/Virage90Right.png";
    spriteRotationOffset = 0.0f;
    angles = { 45.0f / 4, 45.0f / 4,  45.0f / 4, 45.0f / 4, 45.0f / 4, 45.0f / 4,  45.0f / 4, 45.0f / 4 };
    lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage, lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
}

// Virage_90left
Virage_90left::Virage_90left() {
    id = VIRAGE_90LEFT;
    pos = 0;
    spritePath = "trackPieces/versionPNG/Virage90Left.png";
    spriteRotationOffset = 42.5f;
    angles = { -45.0f / 4, -45.0f / 4,  -45.0f / 4, -45.0f / 4, - 45.0f / 4, -45.0f / 4,  -45.0f / 4, -45.0f / 4 };
    lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage, lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
}

// Straight
Straight::Straight() {
    id = STRAIGHT;
    pos = 0;
	spritePath = "trackPieces/versionPNG/Straight.png";
    spriteRotationOffset = 0.0f;
    angles = { 0, 0, 0, 0};
    lengths = { 10, 10, 10, 10};
}

// StartLine
StartLine::StartLine() {
    id = STARTLINE;
    pos = 0;
    angles = { 0 };
    lengths = { 20 };
}

// Garage
Garage::Garage() {
    id = GARAGE;
    pos = 0;
    angles = {};
    lengths = {};
}

// Pit
Pit::Pit() {
    id = PIT;
    pos = 0;
    angles = {};
    lengths = {};
}

// Grandstand
Grandstand::Grandstand() {
    id = GRANDSTAND;
    pos = 0;
    angles = {};
    lengths = {};
}

// Bridges
Bridges::Bridges() {
    id = BRIDGES;
    pos = 0;
    angles = {};
    lengths = {};
}

// Track implementation
Track::Track()
    : startAngle(0)
    , currentAngle(0)
    , currentPos(0, 0)
{
    // Start with just the starting position
    centerLine.push_back(currentPos);

	trackWidth = 40.0f; // Default track width
}

Track::Track(std::vector<int> listPieces)
    : piecesIntList(listPieces)
    , startAngle(0)
    , currentAngle(0)
    , currentPos(0, 0)
    
{
    // Start with the origin
    centerLine.push_back(currentPos);
    trackWidth = 40.0f;
    // Build track from piece list
    for (int pieceId : piecesIntList) {
        calculAngLen(pieceId);
    }

    // Calculate edges after building centerline
    calculateTrackEdges();
}

void Track::calculAngLen(int index)
{
    TrackPieces* piece = nullptr;

    // Create the appropriate piece based on ID
    switch (index) {
    case VIRAGE_45RIGHT:
        piece = new Virage_45right();
        break;
    case VIRAGE_45LEFT:
        piece = new Virage_45left();
        break;
    case VIRAGE_90RIGHT:
        piece = new Virage_90right();
        break;
    case VIRAGE_90LEFT:
        piece = new Virage_90left();
        break;
    case STRAIGHT:
        piece = new Straight();
        break;
    case STARTLINE:
        piece = new StartLine();
        break;
    default:
        std::cerr << "Unknown piece type: " << index << std::endl;
        return;
    }

    if (!piece) return;

    // Get angles and lengths from the piece
    std::vector<float> angles = piece->getAngles();
    std::vector<float> lengths = piece->getLengths();

    // Generate track points
    for (size_t i = 0; i < angles.size() && i < lengths.size(); i++) {
        currentAngle += angles[i];
        currentPos = move(currentPos, currentAngle, lengths[i]);
        centerLine.push_back(currentPos);
    }

    delete piece;
}


void Track::calculateTrackEdges()
{
    trackEdges.left.clear();
    trackEdges.right.clear();

    if (centerLine.size() < 2) return;

    float halfWidth = trackWidth * 0.5f;

    for (size_t i = 0; i < centerLine.size(); i++) {
        QVector2D dir;

        if (i == 0) {
            // First point - use direction to next point
            dir = (centerLine[1] - centerLine[0]).normalized();
        }
        else if (i == centerLine.size() - 1) {
            // Last point - use direction from previous point
            dir = (centerLine[i] - centerLine[i - 1]).normalized();
        }
        else {
            // Middle point - average direction for smooth turns
            QVector2D dir1 = (centerLine[i] - centerLine[i - 1]).normalized();
            QVector2D dir2 = (centerLine[i + 1] - centerLine[i]).normalized();
            dir = (dir1 + dir2).normalized();
        }

        QVector2D normal = perpendicular(dir);

        trackEdges.left.push_back(centerLine[i] + normal * halfWidth);
        trackEdges.right.push_back(centerLine[i] - normal * halfWidth);
    }
}


bool Track::saveToFile(const std::string& filename) const
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }

    // Write header
    file << "TRACK_V1\n";

    // Write track width
    file << "TRACK_WIDTH " << trackWidth << "\n";

    // Write start angle
    file << "START_ANGLE " << startAngle << "\n";

    // Write pieces list (this is what we need to reconstruct the track)
    file << "PIECES " << piecesIntList.size() << "\n";
    for (int pieceId : piecesIntList) {
        file << pieceId << "\n";
    }

    // Optional: Write centerline for verification
    file << "CENTERLINE " << centerLine.size() << "\n";
    for (const auto& point : centerLine) {
        file << point.x() << " " << point.y() << "\n";
    }

    // Optional: Write edges for verification
    file << "LEFT_EDGE " << trackEdges.left.size() << "\n";
    for (const auto& point : trackEdges.left) {
        file << point.x() << " " << point.y() << "\n";
    }

    file << "RIGHT_EDGE " << trackEdges.right.size() << "\n";
    for (const auto& point : trackEdges.right) {
        file << point.x() << " " << point.y() << "\n";
    }

    file.close();
    std::cout << "Track saved successfully to: " << filename << std::endl;
    return true;
}

bool Track::loadFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for reading: " << filename << std::endl;
        return false;
    }

    std::string line;
    std::vector<int> loadedPiecesInt;
	std::vector<TrackPieces*> loadedPieces;
    float loadedTrackWidth = 40;
    float loadedStartAngle = 0;

    // Read header
    std::getline(file, line);
    if (line != "TRACK_V1") {
        std::cerr << "Invalid file format: " << line << std::endl;
        return false;
    }

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "TRACK_WIDTH") {
            iss >> loadedTrackWidth;
        }
        else if (command == "START_ANGLE") {
            iss >> loadedStartAngle;
        }
        else if (command == "PIECES") {
            int count;
            iss >> count;
            loadedPiecesInt.clear();
			loadedPieces.clear();
            for (int i = 0; i < count; i++) {
                std::getline(file, line);
				// Convert piece ID from string to int and store
                int pieceId = std::stoi(line);
                loadedPiecesInt.push_back(pieceId);
				
                TrackPieces* piece = nullptr;;
                switch (pieceId) {
                case VIRAGE_45RIGHT:
                    piece = new Virage_45right();
                    break;
                case VIRAGE_45LEFT:
                    piece = new Virage_45left();
                    break;
                case VIRAGE_90RIGHT:
                    piece = new Virage_90right();
                    break;
                case VIRAGE_90LEFT:
                    piece = new Virage_90left();
                    break;
                case STRAIGHT:
                    piece = new Straight();
                    break;
                case STARTLINE:
                    piece = new StartLine();
                    break;
                };
				loadedPieces.push_back(piece);
            }
        }
        else if (command == "CENTERLINE" || command == "LEFT_EDGE" || command == "RIGHT_EDGE") {
            // Skip these sections - we'll regenerate from pieces
            int count;
            iss >> count;
            for (int i = 0; i < count; i++) {
                std::getline(file, line);
            }
        }
    }

    file.close();

    if (loadedPiecesInt.empty()) {
        std::cerr << "No pieces found in file" << std::endl;
        return false;
    }

    // Reconstruct the track from loaded pieces
    piecesIntList = loadedPiecesInt;
    pieces = loadedPieces;
    trackWidth = loadedTrackWidth;
    startAngle = loadedStartAngle;
    currentAngle = startAngle;
    currentPos = QVector2D(0, 0);

    centerLine.clear();
    centerLine.push_back(currentPos);

    for (int pieceId : piecesIntList) {
        calculAngLen(pieceId);
    }

    calculateTrackEdges();

    std::cout << "Track loaded successfully from: " << filename << std::endl;
    return true;
}



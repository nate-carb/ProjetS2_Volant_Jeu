#include "Track.h"
#include <fstream>
#include <sstream>
#include <QDebug>

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

// stepAngleDeg = angle turned per step, radius = turn radius
float lenghtForStep(float stepAngleDeg, float radius) {
    float rad = std::abs(stepAngleDeg) * (3.14159265f / 180.0f);
    return 2.0f * radius * std::sin(rad / 2.0f);
}
// Arc length of one step = radius * stepAngle_in_radians (for the straight)
float arcLengthPerStep(float stepAngleDeg, float radius) {
    return radius * (stepAngleDeg * 3.14159265f / 180.0f);
}
// Virage_45right
Virage_45right::Virage_45right() {
    id = VIRAGE_45RIGHT;
    pos = 0;
    spritePath = "trackPieces/versionPNG/Virage45Right.png";
    spriteRotationOffset = 0.0f;
	stepAngleDeg = 45.0f / 4;
	float lengthAngleVirage = lenghtForStep(stepAngleDeg, getTurnRadius());
    angles = { stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg };
	lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
    /*
    //angles = { 0, 45.0f / 4, 45.0f / 4,  45.0f / 4, 45.0f / 4, 0};
    angles = { 45.0f / 4, 45.0f / 4,  45.0f / 4, 45.0f / 4 };
    //lengths = { 0, 10, 10, 10, 10, 0 };
    lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
    */
}

// Virage_45left
Virage_45left::Virage_45left() {
    id = VIRAGE_45LEFT;
    pos = 0;
    spritePath = "trackPieces/versionPNG/Virage45Left.png";
    stepAngleDeg = -45.0f / 4;
    float lengthAngleVirage = lenghtForStep(stepAngleDeg, getTurnRadius());
    angles = { stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg };
    lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };

    ////angles = {0, -45.0f / 4, -45.0f / 4,  -45.0f / 4, -45.0f / 4, 0 };
    //angles = { -45.0f / 4, -45.0f / 4,  -45.0f / 4, -45.0f / 4 };
    ////lengths = {0, 10, 10, 10, 10, 0 };
    //lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
}

// Virage_90right
Virage_90right::Virage_90right() {
    id = VIRAGE_90RIGHT;
    pos = 0;
    spritePath = "trackPieces/versionPNG/Virage90Right.png";
    spriteRotationOffset = 0.0f;
    stepAngleDeg = 45.0f / 4;
    float lengthAngleVirage = lenghtForStep(stepAngleDeg, getTurnRadius());
    angles = { stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg };
    lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage, lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
    //angles = { 45.0f / 4, 45.0f / 4,  45.0f / 4, 45.0f / 4, 45.0f / 4, 45.0f / 4,  45.0f / 4, 45.0f / 4 };
    //lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage, lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
}

// Virage_90left
Virage_90left::Virage_90left() {
    id = VIRAGE_90LEFT;
    pos = 0;
    spritePath = "trackPieces/versionPNG/Virage90Left.png";
    spriteRotationOffset = 42.5f;
    stepAngleDeg = -45.0f / 4;
    float lengthAngleVirage = lenghtForStep(stepAngleDeg, getTurnRadius());
    angles = { stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg };
    lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage, lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
    //angles = { -45.0f / 4, -45.0f / 4,  -45.0f / 4, -45.0f / 4, - 45.0f / 4, -45.0f / 4,  -45.0f / 4, -45.0f / 4 };
    //lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage, lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
}

// Straight
Straight::Straight() {
    id = STRAIGHT;
    pos = 0;
	spritePath = "trackPieces/versionPNG/Straight.png";
    spriteRotationOffset = 0.0f;
    
    // This makes one straight piece geometrically equivalent to one 45° curve piece
    //float totalLength = 1.0f * getTurnRadius() * std::sin(22.5f * 3.14159265f / 180.0f);

    //angles = { 0.0f };
    //lengths = { getTrackWidth()};
    // Match the arc length of 4 steps of a 45° turn at TURN_RADIUS
    //float unitLength = arcLengthPerStep(45.0f / 4, getTurnRadius()); // one step unit
    //float straightLength = unitLength * 2  ; // tune the multiplier to match your sprite

    //angles = { 0, 0, 0, 0 };
    //lengths = { straightLength / 4, straightLength / 4, straightLength / 4, straightLength / 4 };

    angles = { 0, 0, 0, 0};
    lengths = { 10, 10, 10, 10};
 //   stepAngleDeg = 0;
	//float lenghtFromTurnRadius = getTurnRadius()/4; // Straight length based on turn radius
 //   angles = { stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg };
 //   lengths = { lenghtFromTurnRadius , lenghtFromTurnRadius, lenghtFromTurnRadius, lenghtFromTurnRadius };
}

// StartLine
StartLine::StartLine() {
    id = STARTLINE;
    pos = 0;
    angles = { 0 };
    lengths = { 10 };
}


// Pit
Pit::Pit() {
    id = PIT;
    pos = 0;
    float step = 10.0f; // match Straight length
    angles = { 0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f, 
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f };
    lengths = { step, step, step, step,
                step, step, step, step,
                step, step, step, step,
                step, step, step, step,
                step, step, step, step,
                step, step, step, step, 
                step, step, step, step,
                step, step, step, step };
}


//void Track::generatePitLane(int startIndex, int endIndex)
void Track::generatePitLane(int startIndex, int endIndex)
{
    pitStartIndex = startIndex;
    pitEndIndex = endIndex;

    if (pitStartIndex < 0 || pitEndIndex < 0) return;
    if (pitEndIndex >= (int)centerLine.size()) return;

    // Clear all previous pit lane data
    pitLane.centerLine.clear();
    pitLane.edges.left.clear();
    pitLane.edges.right.clear();
    pitLane.entryCurve.clear();
    pitLane.exitCurve.clear();
    pitLane.entryCurveEdges.left.clear();
    pitLane.entryCurveEdges.right.clear();
    pitLane.exitCurveEdges.left.clear();
    pitLane.exitCurveEdges.right.clear();

    float pitOffset = trackWidth * 2.5f; // how far the pit lane is from the main track center
    float halfWidth = trackWidth * 0.5f; // half track width for edge offset
    int   curveSteps = 24;                // number of bezier steps (higher = smoother)

    // ─────────────────────────────────────────────────────────────
    // Compute a stable offset direction for the entire pit section
    // We use the midpoint direction so the pit lane stays parallel
    // to the track even if the track curves slightly in this section
    // ─────────────────────────────────────────────────────────────
    int midIndex = (pitStartIndex + pitEndIndex) / 2;
    QVector2D midDir = (centerLine[midIndex + 1] - centerLine[midIndex]).normalized();
    QVector2D pitNormal = perpendicular(midDir); // consistent right-side normal for whole pit

    // ─────────────────────────────────────────────────────────────
    // Build the pit lane STRAIGHT section
    // Only covers the middle portion of the pit block (pitRatio trims
    // the ends to leave room for the entry/exit curves)
    // ─────────────────────────────────────────────────────────────
    int pitSize = pitEndIndex - pitStartIndex + 1;
    int pitRatio = pitSize / 3; // trim 1/3 from each end for curve space

    for (int i = (pitStartIndex + pitRatio); i <= (pitEndIndex - pitRatio); i++) {
        // Compute local track direction at this point for edge normals
        QVector2D dir;
        if (i == 0)
            dir = (centerLine[1] - centerLine[0]).normalized();
        else if (i == (int)centerLine.size() - 1)
            dir = (centerLine[i] - centerLine[i - 1]).normalized();
        else {
            QVector2D d1 = (centerLine[i] - centerLine[i - 1]).normalized();
            QVector2D d2 = (centerLine[i + 1] - centerLine[i]).normalized();
            dir = (d1 + d2).normalized();
        }

        // Offset the centerline point by pitOffset using the stable pitNormal
        QVector2D pitPoint = centerLine[i] - pitNormal * pitOffset;
        pitLane.centerLine.push_back(pitPoint);

        // Left/right edges use local direction for correct perpendicular
        pitLane.edges.left.push_back(pitPoint + perpendicular(dir) * halfWidth);
        pitLane.edges.right.push_back(pitPoint - perpendicular(dir) * halfWidth);
    }

    if (pitLane.centerLine.size() < 2) return;

    // ─────────────────────────────────────────────────────────────
    // ENTRY CURVE
    // Cubic bezier from the main track CENTERLINE to the first point
    // of the pit straight. Starting from the centerline (not the edge)
    // means the curve visually peels away from the main road smoothly.
    // ─────────────────────────────────────────────────────────────

    // Entry starts at the main track centerline point
    QVector2D entryStartDir = (centerLine[pitStartIndex + 1] - centerLine[pitStartIndex]).normalized();
    QVector2D entryStart = centerLine[pitStartIndex]; // START ON CENTERLINE

    // Entry ends exactly at the first point of the pit straight
    QVector2D entryEnd = pitLane.centerLine.front();
    QVector2D entryEndDir = (pitLane.centerLine[1] - pitLane.centerLine[0]).normalized();

    // Bezier control points extend along entry/exit directions for a smooth S-curve
    float cpDist = (entryEnd - entryStart).length() * 0.5f;
    QVector2D cp1 = entryStart + entryStartDir * cpDist; // tangent at start
    QVector2D cp2 = entryEnd - entryEndDir * cpDist; // tangent at end

    // Sample the cubic bezier curve at curveSteps intervals
    for (int i = 0; i <= curveSteps; i++) {
        float t = (float)i / curveSteps;
        float t2 = t * t, t3 = t2 * t;
        float u = 1.0f - t;
        float u2 = u * u, u3 = u2 * u;
        // Standard cubic bezier: B(t) = (1-t)^3*P0 + 3(1-t)^2*t*P1 + 3(1-t)*t^2*P2 + t^3*P3
        QVector2D point = entryStart * u3
            + cp1 * (3 * u2 * t)
            + cp2 * (3 * u * t2)
            + entryEnd * t3;
        pitLane.entryCurve.push_back(point);
    }

    // Force exact endpoints to eliminate floating point gaps
    pitLane.entryCurve.front() = entryStart;
    pitLane.entryCurve.back() = entryEnd;

    // Build entry curve edges using the tangent direction at each curve point
    for (size_t i = 0; i < pitLane.entryCurve.size(); i++) {
        QVector2D dir;
        if (i == 0)
            dir = (pitLane.entryCurve[1] - pitLane.entryCurve[0]).normalized();
        else if (i == pitLane.entryCurve.size() - 1)
            dir = (pitLane.entryCurve[i] - pitLane.entryCurve[i - 1]).normalized();
        else
            // Average of incoming and outgoing directions for smooth normals
            dir = ((pitLane.entryCurve[i] - pitLane.entryCurve[i - 1]) +
                (pitLane.entryCurve[i + 1] - pitLane.entryCurve[i])).normalized();
        QVector2D n = perpendicular(dir);
        pitLane.entryCurveEdges.left.push_back(pitLane.entryCurve[i] + n * halfWidth);
        pitLane.entryCurveEdges.right.push_back(pitLane.entryCurve[i] - n * halfWidth);
    }

    // Force first edge to align with main track centerline (zero width at start)
    // This makes the curve appear to "peel off" from the main road
    //pitLane.entryCurveEdges.left.front() = entryStart; // both edges start at center = zero width
    //pitLane.entryCurveEdges.right.front() = entryStart;

    // Force last edge to connect seamlessly to pit straight edges
    pitLane.entryCurveEdges.left.back() = pitLane.edges.left.front();
    pitLane.entryCurveEdges.right.back() = pitLane.edges.right.front();

    // ─────────────────────────────────────────────────────────────
    // EXIT CURVE
    // Cubic bezier from the last point of the pit straight back to
    // the main track CENTERLINE. Merges back to a single point on
    // the centerline for a smooth rejoining effect.
    // ─────────────────────────────────────────────────────────────

    // Exit starts exactly at the last point of the pit straight
    QVector2D exitStart = pitLane.centerLine.back();
    QVector2D exitStartDir = (pitLane.centerLine.back() - pitLane.centerLine[pitLane.centerLine.size() - 2]).normalized();

    // Exit ends at the main track centerline point
    QVector2D exitEndDir = (centerLine[pitEndIndex] - centerLine[pitEndIndex - 1]).normalized();
    QVector2D exitEnd = centerLine[pitEndIndex]; // END ON CENTERLINE

    float cpDistExit = (exitEnd - exitStart).length() * 0.5f;
    QVector2D cp3 = exitStart + exitStartDir * cpDistExit; // tangent at start
    QVector2D cp4 = exitEnd - exitEndDir * cpDistExit; // tangent at end

    // Sample the exit bezier curve
    for (int i = 0; i <= curveSteps; i++) {
        float t = (float)i / curveSteps;
        float t2 = t * t, t3 = t2 * t;
        float u = 1.0f - t;
        float u2 = u * u, u3 = u2 * u;
        QVector2D point = exitStart * u3
            + cp3 * (3 * u2 * t)
            + cp4 * (3 * u * t2)
            + exitEnd * t3;
        pitLane.exitCurve.push_back(point);
    }

    // Force exact endpoints to eliminate floating point gaps
    pitLane.exitCurve.front() = exitStart;
    pitLane.exitCurve.back() = exitEnd;

    // Build exit curve edges using tangent direction at each point
    for (size_t i = 0; i < pitLane.exitCurve.size(); i++) {
        QVector2D dir;
        if (i == 0)
            dir = (pitLane.exitCurve[1] - pitLane.exitCurve[0]).normalized();
        else if (i == pitLane.exitCurve.size() - 1)
            dir = (pitLane.exitCurve[i] - pitLane.exitCurve[i - 1]).normalized();
        else
            dir = ((pitLane.exitCurve[i] - pitLane.exitCurve[i - 1]) +
                (pitLane.exitCurve[i + 1] - pitLane.exitCurve[i])).normalized();
        QVector2D n = perpendicular(dir);
        pitLane.exitCurveEdges.left.push_back(pitLane.exitCurve[i] + n * halfWidth);
        pitLane.exitCurveEdges.right.push_back(pitLane.exitCurve[i] - n * halfWidth);
    }

    // Force first edge to connect seamlessly to pit straight edges
    pitLane.exitCurveEdges.left.front() = pitLane.edges.left.back();
    pitLane.exitCurveEdges.right.front() = pitLane.edges.right.back();

    // Force last edge to merge back to centerline point (zero width at end)
    // This makes the curve appear to "merge into" the main road
    //pitLane.exitCurveEdges.left.back() = exitEnd; // both edges end at center = zero width
    //pitLane.exitCurveEdges.right.back() = exitEnd;

    pitLane.isValid = true;
}
void Track::addDecor(int decorType, int decorIndexList)
{
    switch (decorType) {
        case GRANDSTAND_INDEX:
            decors.push_back(new Grandstand(QVector2D(50, 50), 0.0f));
            decors.back()->selectModel(decorIndexList);
            qDebug() << "Decor type: GRANDSTAND" ;
            break;

        case GARAGE_INDEX:
            decors.push_back(new Garage(QVector2D(50, 50), 0.0f));
            decors.back()->selectModel(decorIndexList);
            qDebug() << "Decor type: GARAGE";
            break;

        case TREES_INDEX:
            decors.push_back(new TREES(QVector2D(50, 50), 0.0f));
            decors.back()->selectModel(decorIndexList);
            qDebug() << "Decor type: TREES";
            break;

            //case NOSPECIFICDECOR_INDEX:
            //    decors.push_back(new NoSpecificDecor(QVector2D(0, 0), 0.0f));
            //    decors.back()->selectModel(decorIndexList);
            //break;
        default:
            qDebug() << "Invalid decor type: " << decorType;
            break;
    }
}
//------------------------------------
//--- Decor pieces implementations ---
//------------------------------------
DecorPieces::~DecorPieces() {}

DecorPieces::DecorPieces()
{
    info.pos = QVector2D(0, 0);
    info.angle = 0.0f;
	info.scale = 30.0f;
}





Grandstand::Grandstand(QVector2D p, float a)
{
    info.pos = p;
    info.angle = a;
    info.width = 1.0f; // need to tune this based on the 3D model size
    info.length = 1.0f; // need to tune this based on the 3D model size
	info.modelType = GRANDSTAND_INDEX;
	info.modelIndex = 0; // Default to the first model in the list

    modelList = {
        "/3dModels/dae/grandStand.dae",
        "/3dModels/dae/grandStandCovered.dae",
        "/3dModels/dae/grandStandAwning.dae",
        "/3dModels/dae/grandStandRound.dae",
        "/3dModels/dae/grandStandCoveredRound.dae"
    };
    info.modelPath = modelList[0]; // Default to the first model for the garage
}

void Grandstand::selectModel(int modelNum)
{
	//Model selection based on the provided model number. If the number is out of range, it defaults to the first model.
    if (modelNum >= 0 && modelNum < (int)modelList.size()) {
        info.modelPath = modelList[modelNum];
		info.modelIndex = modelNum;
    }
    else {
        qDebug() << "Invalid model number for grandstand: " << modelNum;
        info.modelPath = modelList[0]; // Set to empty or a default model
        info.modelIndex = 0; // Default to first model if invalid
    }
    switch (info.modelIndex) {
        case 0: // grandStand.obj
            info.width = 1.0f; // Adjust width for this model
			info.height = 0.9f; // Adjust height for this model
            info.length = 1.0f; // Adjust length for this model
            break;
        case 1: // grandStandCovered.obj
            info.width = 1.0f; // Adjust width for this model
            info.height = 1.19f; // Adjust height for this model
            info.length = 1.02f; // Adjust length for this model
            break;
        case 2: // grandStandAwning.obj
            info.width = 1.0f; // Adjust width for this model
            info.height = 1.39f; // Adjust height for this model
            info.length = 1.0f; // Adjust length for this modell
            break;
        case 3: // grandStandRound.obj
            info.width = 1.64f; // Adjust width for this model
            info.height = 0.9f; // Adjust height for this model
            info.length = 1.64f; // Adjust length for this model
            break;
        case 4: // grandStandCoveredRound.obj
            info.width = 1.64f; // Adjust width for this model
            info.height = 1.19f; // Adjust height for this model
            info.length = 1.64f; // Adjust length for this model
            break;
        default:
			info.width = 1.0f; // Default width
			info.height = 0.9f; // Default height
			info.length = 1.0f; // Default length
            break;
    }

}

Garage::Garage(QVector2D p, float a)
{
    info.pos = p;
    info.angle = a;
    info.width = 1.0f;
    info.height = 0.7f;
    info.length = 1.1f;
	info.modelType = GARAGE_INDEX;
	info.modelIndex = 0; // Default to the first model in the list
    

    modelList = {
        "/3dModels/dae/pitsGarage.dae",
        "/3dModels/dae/pitsGarageClosed.dae",
        "/3dModels/dae/pitsGarageCorner.dae"
    };
    info.modelPath = modelList[0]; // Default to the first model for the garage
}

void Garage::selectModel(int modelNum)
{
    //Model selection based on the provided model number. If the number is out of range, it defaults to the first model.
    if (modelNum >= 0 && modelNum < (int)modelList.size()) {
        info.modelPath = modelList[modelNum];
		info.modelIndex = modelNum;
    }
    else {
        qDebug() << "Invalid model number for grandstand: " << modelNum;
        info.modelPath = modelList[0]; // Set to empty or a default model
        info.modelIndex = 0; // Default to first model if invalid
    }

    switch (info.modelIndex) {
    case 0: // pitsGarage.obj
        info.width = 1.0f; // Adjust width for this model
        info.height = 0.7f; // Adjust height for this model
        info.length = 1.1f; // Adjust length for this model
        break;
    case 1: // pitsGarageClosed.obj
        info.width = 1.0f; // Adjust width for this model
        info.height = 1.7f; // Adjust height for this model
        info.length = 1.1f; // Adjust length for this model
        break;
    case 2: // pitsGarageCorner.obj
        info.width = 1.05f; // Adjust width for this model
        info.height = 1.7f; // Adjust height for this model
        info.length = 1.0f; // Adjust length for this modell
        break;
    default:
        info.width = 1.0f; // Default width
        info.height = 0.7f; // Default height
        info.length = 1.1f; // Default length
        break;
    }

}
TREES::TREES(QVector2D p, float a)
{
    info.pos = p;
    info.angle = a;
    info.width = 0.25f; // need to tune this based on the 3D model size
    info.length = 0.29f; // need to tune this based on the 3D model size
	info.modelType = TREES_INDEX;
	info.modelIndex = 0; // Default to the first model in the list

    modelList = {
        "/3dModels/dae/treeSmall.dae",
        "/3dModels/dae/treeLarge.dae"
    };
    info.modelPath = modelList[0]; // Default to the first model for the garage
}

void TREES::randomModel()
{
    info.modelIndex = rand() % modelList.size();
	selectModel(info.modelIndex);
}

void TREES::selectModel(int modelNum)
{
    //Model selection based on the provided model number. If the number is out of range, it defaults to the first model.
    if (modelNum >= 0 && modelNum < (int)modelList.size()) {
        info.modelPath = modelList[modelNum];
		info.modelIndex = modelNum;
    }
    else {
        qDebug() << "Invalid model number for grandstand: " << modelNum;
        info.modelPath = modelList[0]; // Set to empty or a default model
		info.modelIndex = 0; // Default to first model if invalid
    }
    if (info.modelIndex == 0) {
        info.width = 0.25f; // Adjust width for rounded models
        info.length = 0.29f; // Adjust length for rounded models
		info.height = 1.07f; // Adjust height for rounded models
    }
    else {
        info.width = 0.36f; // Adjust width for other models
        info.length = 0.41f; // Adjust length for other models
		info.height = 1.51f; // Adjust height for other models
    }

}
NoSpecificDecor::NoSpecificDecor(QVector2D p, float a)
{
    info.pos = p;
    info.angle = a;
    info.width = 1.0f; // arbitrary small size for non-specific decor
    info.length = 1.0f;
	info.modelType = NOSPECIFICDECOR_INDEX;
	info.modelIndex = 0; // Default to the first model in the list

    modelList = {
        "decorPieces/NonSpecificModel1.obj",
        "decorPieces/NonSpecificModel2.obj",
        "decorPieces/NonSpecificModel3.obj"
    };
    info.modelPath = modelList[0]; // Default to the first model for the garage
}
void NoSpecificDecor::selectModel(int modelNum)
{
    //Model selection based on the provided model number. If the number is out of range, it defaults to the first model.
    if (modelNum >= 0 && modelNum < (int)modelList.size()) {
        info.modelPath = modelList[modelNum];
        info.modelIndex = modelNum;
    }
    else {
        qDebug() << "Invalid model number for non-specific decor: " << modelNum;
        info.modelPath = modelList[0]; // Set to empty or a default model
        info.modelIndex = 0; // Default to first model if invalid
    }
    // For non-specific decor, we can keep the size constant or add some variation if desired
    info.width = 1.0f; // Default width
    info.length = 1.0f; // Default length
    info.height = 1.0f; // Default height
}
//---------------------------
// ---Track implementation---
//---------------------------
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
//------------------------------------------------
// --- Track building and modification methods ---
//------------------------------------------------
void Track::addCurveSegment()
{
    TrackSegment seg;
    seg.type = TrackSegmentType::CURVE_TRACK;

    if (trackSegments.empty()) {
        seg.start = QVector2D(0, 0);
        seg.end = QVector2D(200, 0);
    }
    else {
        seg.start = trackSegments.back().end;
        QVector2D lastDir = (trackSegments.back().end -
            trackSegments.back().start).normalized();
        seg.end = seg.start + lastDir * 200.0f;
    }

    QVector2D dir = (seg.end - seg.start).normalized();
    QVector2D perp = QVector2D(-dir.y(), dir.x());
    seg.cp1 = seg.start + dir * 0.33f * 200.0f + perp * 50.0f;
    seg.cp2 = seg.start + dir * 0.66f * 200.0f + perp * 50.0f;

    trackSegments.push_back(seg);
    buildFromSegments(); // fills centerLine + trackEdges
}

void Track::addStraightSegment()
{
    TrackSegment seg;
    seg.type = TrackSegmentType::STRAIGHT_TRACK;

    if (trackSegments.empty()) {
        seg.start = QVector2D(0, 0);
        seg.end = QVector2D(200, 0);
    }
    else {
        seg.start = trackSegments.back().end;
        QVector2D lastDir = (trackSegments.back().end -
            trackSegments.back().start).normalized();
        seg.end = seg.start + lastDir * 200.0f;
    }

    seg.cp1 = seg.start;
    seg.cp2 = seg.end;

    trackSegments.push_back(seg);
	qDebug() << "Added straight segment from" << seg.start << "to" << seg.end;
    buildFromSegments(); // fills centerLine + trackEdges
}

void Track::removeLastSegment()
{
    if (trackSegments.empty()) return;
    trackSegments.pop_back();
    buildFromSegments(); // refills centerLine + trackEdges
}

// Adds a pit segment that extends from the last track segment, ensuring it's long enough for a pit lane. The actual pit lane geometry is generated in buildFromSegments() when it processes this segment type.
void Track::addPitSegment()
{
    TrackSegment seg;
    seg.type = TrackSegmentType::PIT_TRACK;

    if (trackSegments.empty()) {
        seg.start = QVector2D(0, 0);
        seg.end = QVector2D(400, 0); // pit lane needs to be longer than normal straight
    }
    else {
        seg.start = trackSegments.back().end;
        QVector2D lastDir = (trackSegments.back().end -
            trackSegments.back().start).normalized();
        seg.end = seg.start + lastDir * 400.0f;
    }

    seg.cp1 = seg.start;
    seg.cp2 = seg.end;

    trackSegments.push_back(seg);
    buildFromSegments();
}

void Track::buildFromSegments()
{
    centerLine.clear();
    trackEdges.left.clear();
    trackEdges.right.clear();
	checkpoints.clear();

    if (trackSegments.empty()) return;

    int steps = 20;

    for (int si = 0; si < (int)trackSegments.size(); si++) {
        const TrackSegment& seg = trackSegments[si];
        int startI = (si == 0) ? 0 : 1;

        // Record pit start index BEFORE adding points
        int segStartIndex = centerLine.size();

        for (int i = startI; i <= steps; i++) {
            float t = (float)i / steps;
            float u = 1.0f - t;

            QVector2D point;
            if (seg.type == TrackSegmentType::STRAIGHT_TRACK ||
                seg.type == TrackSegmentType::PIT_TRACK) {
                point = seg.start * (1.0f - t) + seg.end * t;
            }
            else {
                point = seg.start * (u * u * u)
                    + seg.cp1 * (3 * u * u * t)
                    + seg.cp2 * (3 * u * t * t)
                    + seg.end * (t * t * t);
            }

            QVector2D tangent;
            if (i < steps) {
                float t2 = (float)(i + 1) / steps;
                float u2 = 1.0f - t2;
                QVector2D next;
                if (seg.type == TrackSegmentType::STRAIGHT_TRACK ||
                    seg.type == TrackSegmentType::PIT_TRACK) {
                    next = seg.start * (1.0f - t2) + seg.end * t2;
                }
                else {
                    next = seg.start * (u2 * u2 * u2)
                        + seg.cp1 * (3 * u2 * u2 * t2)
                        + seg.cp2 * (3 * u2 * t2 * t2)
                        + seg.end * (t2 * t2 * t2);
                }
                tangent = (next - point).normalized();
            }
            else if (!centerLine.empty()) {
                tangent = (point - centerLine.back()).normalized();
            }
            else {
                tangent = QVector2D(1, 0);
            }

            QVector2D normal(-tangent.y(), tangent.x());
            centerLine.push_back(point);
            trackEdges.left.push_back(point + normal * (trackWidth / 2.0f));
            trackEdges.right.push_back(point - normal * (trackWidth / 2.0f));
        }

        // Generate pit lane for pit segments using correct indices
        if (seg.type == TrackSegmentType::PIT_TRACK) {
            int segEndIndex = centerLine.size() - 1;
            generatePitLane(segStartIndex, segEndIndex);
        }
        else {
			createCheckpointAtSegment(); // place checkpoint at the end of this segment
        }
    }

    qDebug() << "buildFromSegments:" << centerLine.size() << "centerline points"
        << trackEdges.left.size() << "left edge points";
}
//void Track::createStartLine()
//{
//    Checkpoint temp;
//    temp.centerlineIndex = 0; // place start line at the very beginning of the track
//    temp.leftEdgeIndex = 0;
//    temp.rightEdgeIndex = 0;
//
//    temp.angle = startAngle; // use track's starting angle for orientation
//	checkpoints.push_back(temp);
//}

//--------------------
// --- Checkpoint  ---
//--------------------
void Track::createCheckpointAtSegment()
{
    if (centerLine.empty()) return;
    if (trackEdges.left.empty()) return;

    int clIndex = centerLine.size() - 1;

    // Get forward direction from last two centerline points
    QVector2D forward;
    if (clIndex > 0)
        forward = (centerLine[clIndex] - centerLine[clIndex - 1]).normalized();
    else
        forward = QVector2D(1, 0);

    CheckpointData cp;
    cp.left = trackEdges.left[clIndex];
    cp.right = trackEdges.right[clIndex];
    cp.forward = forward; // SAVE direction
    cp.centerLineIndex = clIndex;
    cp.triggered = false;

    checkpoints.push_back(cp);
}
// Checks if the car is within a certain distance (threshold) of the line segment defined by pointA and pointB. This is used to determine if the car has crossed a checkpoint.
bool Track::isCarBetweenPoints(const QVector2D& carPos,
    const QVector2D& pointA,
    const QVector2D& pointB,
    float threshold) const
{
    // Vector from A to B (the checkpoint line)
    QVector2D AB = pointB - pointA;
    QVector2D AP = carPos - pointA;

    float ab2 = QVector2D::dotProduct(AB, AB);
    if (ab2 == 0.0f) return false;

    // Project car onto the line AB
    float t = QVector2D::dotProduct(AP, AB) / ab2;

    // t must be between 0 and 1 to be between the two points
    if (t < 0.0f || t > 1.0f) return false;

    // Distance from car to the line
    QVector2D closest = pointA + AB * t;
    float dist = (carPos - closest).length();

    return dist <= threshold;
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
    case PIT:
        piece = new Pit();
		break;
    default:
        std::cerr << "Unknown piece type: " << index << std::endl;
        return;
    }

    if (!piece) return;

    // Get angles and lengths from the piece
    std::vector<float> angles = piece->getAngles();
    std::vector<float> lengths = piece->getLengths();

    
    // Record where the piece starts in the centerline
    

    piece->setStartIndex(centerLine.size() - 1);

    // Generate track points
    for (size_t i = 0; i < angles.size() && i < lengths.size(); i++) {
        currentAngle += angles[i];
        currentPos = move(currentPos, currentAngle, lengths[i]);
        centerLine.push_back(currentPos);
    }

    piece->setEndIndex(centerLine.size() - 1);
    

    if (piece->getId() == PIT) {
        generatePitLane(piece->getStartIndex(), piece->getEndIndex()); // auto-generate parallel lane
		//generatePitLane(); // auto-generate parallel lane
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

bool Track::isClosed() const
{
    if (centerLine.size() < 2) return false;
    return (centerLine.back() - centerLine.front()).length() < trackWidth;
}

float Track::getClosureGap() const
{
    if (centerLine.size() < 2) return -1.0f;
    return (centerLine.back() - centerLine.front()).length();
}

// Closes the track by adding a smooth bezier curve from the end back to the start
void Track::closeTrack()
{
    if (isClosed()) return;
    if (centerLine.size() < 2) return;

    QVector2D start = centerLine.front();
    QVector2D end = centerLine.back();

    // Compute tangents at start and end for smooth connection
    QVector2D startTangent = (centerLine[1] - centerLine[0]).normalized();
    QVector2D endTangent = (centerLine.back() -
        centerLine[centerLine.size() - 2]).normalized();

    float dist = (end - start).length();

    // Control points extend along tangent directions
    QVector2D cp1 = end + endTangent * (dist * 0.4f);
    QVector2D cp2 = start - startTangent * (dist * 0.4f);

    // Sample the closing bezier and append to centerline/edges
    int steps = 20;
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / steps;
        float u = 1.0f - t;

        // Cubic bezier: end → cp1 → cp2 → start
        QVector2D point = end * (u * u * u)
            + cp1 * (3 * u * u * t)
            + cp2 * (3 * u * t * t)
            + start * (t * t * t);

        // Tangent for edge normals
        QVector2D tangent;
        if (i < steps) {
            float t2 = (float)(i + 1) / steps;
            float u2 = 1.0f - t2;
            QVector2D next = end * (u2 * u2 * u2)
                + cp1 * (3 * u2 * u2 * t2)
                + cp2 * (3 * u2 * t2 * t2)
                + start * (t2 * t2 * t2);
            tangent = (next - point).normalized();
        }
        else {
            tangent = startTangent;
        }

        QVector2D normal(-tangent.y(), tangent.x());
        centerLine.push_back(point);
        trackEdges.left.push_back(point + normal * (trackWidth / 2.0f));
        trackEdges.right.push_back(point - normal * (trackWidth / 2.0f));
    }

    qDebug() << "Track closed - gap was:" << (int)dist << "units";
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
	// Write track segments (for reconstructing bezier curves and centerline)
    file << "TRACK_SEGMENTS " << trackSegments.size() << "\n";
    for (const auto& s : trackSegments) {
        file << (int)s.type << " "
            << s.start.x() << " " << s.start.y() << " "
            << s.end.x() << " " << s.end.y() << " "
            << s.cp1.x() << " " << s.cp1.y() << " "
            << s.cp2.x() << " " << s.cp2.y() << "\n";
    }
	// Write decors list
	file << "DECORS " << decors.size() << "\n";
	for (const auto& decor : decors) {
		file << decor->getInfo().modelType << " " << decor->getInfo().modelIndex << " " << decor->getInfo().pos.x() << " " << decor->getInfo().pos.y() << " " << decor->getInfo().angle << "\n";
	}

    // Write bezier curves for walls
    file << "BEZIER_CURVES " << bezierCurves.size() << "\n";
    for (const auto& c : bezierCurves) {
        file << c.p0.x() << " " << c.p0.y() << " "
            << c.p1.x() << " " << c.p1.y() << " "
            << c.p2.x() << " " << c.p2.y() << " "
            << c.p3.x() << " " << c.p3.y() << "\n";
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

    file << "CHECKPOINTS " << checkpoints.size() << "\n";
    for (const auto& cp : checkpoints) {
        file << cp.left.x() << " " << cp.left.y() << " "
            << cp.right.x() << " " << cp.right.y() << " "
            << cp.forward.x() << " " << cp.forward.y() << " "  
            << cp.centerLineIndex << "\n";
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
	std::vector<DecorPieces*> loadedDecors;
	
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
        else if (command == "TRACK_SEGMENTS") {
            int count;
            iss >> count;
            trackSegments.clear();
            for (int i = 0; i < count; i++) {
                std::getline(file, line);
                std::istringstream segIss(line);
                TrackSegment s;
                int type;
                float sx, sy, ex, ey, c1x, c1y, c2x, c2y;
                segIss >> type >> sx >> sy >> ex >> ey
                    >> c1x >> c1y >> c2x >> c2y;
                s.type = (TrackSegmentType)type;
                s.start = QVector2D(sx, sy);
                s.end = QVector2D(ex, ey);
                s.cp1 = QVector2D(c1x, c1y);
                s.cp2 = QVector2D(c2x, c2y);
                trackSegments.push_back(s);
            }
            buildFromSegments();
        }
        else if (command == "DECORS") {

            int count;
            iss >> count;
            for (int i = 0; i < count; i++) {
                std::getline(file, line);
                std::istringstream decorIss(line);
                int decorType, decorIndex;
                float posX, posY, angle;
                decorIss >> decorType >> decorIndex >> posX >> posY >> angle;
                DecorPieces* decor = nullptr;
                switch (decorType) {
                    case GRANDSTAND_INDEX:
                        decor = new Grandstand(QVector2D(posX, posY), angle);
                        break;
                    case GARAGE_INDEX:
                        decor = new Garage(QVector2D(posX, posY), angle);
                        break;
                    case TREES_INDEX:
                        decor = new TREES(QVector2D(posX, posY), angle);
                        break;
                    //case NOSPECIFICDECOR_INDEX:
                    //    decor = new NoSpecificDecor(QVector2D(posX, posY), angle);
                    //    break;
                    default:
                        qDebug() << "Invalid decor type in file: " << decorType;
                        continue; // skip invalid decor
                }
                if (decor) {
                    decor->selectModel(decorIndex); // set the specific model based on index
                    loadedDecors.push_back(decor);
                }
			}
        
        }
        else if (command == "BEZIER_CURVES") {
            int count;
            iss >> count;
            bezierCurves.clear();
            for (int i = 0; i < count; i++) {
                std::getline(file, line);
                std::istringstream bezierIss(line);
                BezierCurveData c;
                float x0, y0, x1, y1, x2, y2, x3, y3;
                bezierIss >> x0 >> y0 >> x1 >> y1 >> x2 >> y2 >> x3 >> y3;
                c.p0 = QVector2D(x0, y0);
                c.p1 = QVector2D(x1, y1);
                c.p2 = QVector2D(x2, y2);
                c.p3 = QVector2D(x3, y3);
                bezierCurves.push_back(c);
            }
        }
        else if (command == "CHECKPOINTS") {
            int count;
            iss >> count;
            checkpoints.clear();
            for (int i = 0; i < count; i++) {
                std::getline(file, line);
                std::istringstream cpIss(line);
                CheckpointData cp;
                float lx, ly, rx, ry, fx, fy;
                cpIss >> lx >> ly >> rx >> ry >> fx >> fy >> cp.centerLineIndex;
                cp.left = QVector2D(lx, ly);
                cp.right = QVector2D(rx, ry);
                cp.forward = QVector2D(fx, fy);
                cp.triggered = false;
                checkpoints.push_back(cp);
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

	// Basic validation to ensure we have the necessary data to reconstruct the track
    if (loadedPiecesInt.empty() && trackSegments.empty()) {
        std::cerr << "No pieces or segments found in file" << std::endl;
        return false;
    }

    // Reconstruct the track from loaded pieces
    piecesIntList = loadedPiecesInt;
    pieces = loadedPieces;
	decors = loadedDecors;
	
    trackWidth = loadedTrackWidth;
    startAngle = loadedStartAngle;
    currentAngle = startAngle;
    currentPos = QVector2D(0, 0);

    if (!piecesIntList.empty()) {
        // Piece-based track - rebuild from pieces
        centerLine.clear();
        centerLine.push_back(currentPos);
        for (int pieceId : piecesIntList)
            calculAngLen(pieceId);
        calculateTrackEdges();
        closeTrack();
    }
    else if (!trackSegments.empty()) {
        // Segment-based track - already built during parsing
        // just close it
        closeTrack();
    }

    std::cout << "Track loaded successfully from: " << filename << std::endl;
    return true;
}



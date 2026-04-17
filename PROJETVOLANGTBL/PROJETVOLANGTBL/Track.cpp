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

    if (ab2 == 0.0f)
        return (P - A).length();

    float t = QVector2D::dotProduct(AP, AB) / ab2;

    t = std::max(0.0f, std::min(1.0f, t));

    QVector2D closestPoint = A + AB * t;

	// La distance entre P et le point le plus proche sur le segment AB
    return (P - closestPoint).length();
}
bool Track::isVector2DOnTrack(const QVector2D& point) const
{
    float minDist = 300000;
    for (size_t i = 1; i < centerLine.size(); i++) {

        float d = distancePointToSegment(point, centerLine[i - 1], centerLine[i]);
        minDist = std::min(minDist, d); 
    }
    return minDist <= trackWidth / 2.0f; 
}
bool Track::isVector2DOnPitLane(const QVector2D& point, bool onTrack) const
{
    if (onTrack) return false;

    float minDist = 300000;

    // Pit straight
    for (size_t i = 1; i < pitLane.centerLine.size(); i++) {
        float d = distancePointToSegment(point, pitLane.centerLine[i - 1], pitLane.centerLine[i]);
        minDist = std::min(minDist, d);
    }

    // Entry curve
    for (size_t i = 1; i < pitLane.entryCurve.size(); i++) {
        float d = distancePointToSegment(point, pitLane.entryCurve[i - 1], pitLane.entryCurve[i]);
        minDist = std::min(minDist, d);
    }

    // Exit curve
    for (size_t i = 1; i < pitLane.exitCurve.size(); i++) {
        float d = distancePointToSegment(point, pitLane.exitCurve[i - 1], pitLane.exitCurve[i]);
        minDist = std::min(minDist, d);
    }

    return minDist <= trackWidth / 2.0f;
}

TrackPieces::~TrackPieces() {}

float lenghtForStep(float stepAngleDeg, float radius) {
    float rad = std::abs(stepAngleDeg) * (3.14159265f / 180.0f);
    return 2.0f * radius * std::sin(rad / 2.0f);
}

float arcLengthPerStep(float stepAngleDeg, float radius) {
    return radius * (stepAngleDeg * 3.14159265f / 180.0f);
}

Virage_45right::Virage_45right() {
    id = VIRAGE_45RIGHT;
    pos = 0;
    spritePath = "trackPieces/versionPNG/Virage45Right.png";
    spriteRotationOffset = 0.0f;
	stepAngleDeg = 45.0f / 4;
	float lengthAngleVirage = lenghtForStep(stepAngleDeg, getTurnRadius());
    angles = { stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg };
	lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
}

Virage_45left::Virage_45left() {
    id = VIRAGE_45LEFT;
    pos = 0;
    spritePath = "trackPieces/versionPNG/Virage45Left.png";
    stepAngleDeg = -45.0f / 4;
    float lengthAngleVirage = lenghtForStep(stepAngleDeg, getTurnRadius());
    angles = { stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg };
    lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
}

Virage_90right::Virage_90right() {
    id = VIRAGE_90RIGHT;
    pos = 0;
    spritePath = "trackPieces/versionPNG/Virage90Right.png";
    spriteRotationOffset = 0.0f;
    stepAngleDeg = 45.0f / 4;
    float lengthAngleVirage = lenghtForStep(stepAngleDeg, getTurnRadius());
    angles = { stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg };
    lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage, lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
    
}

Virage_90left::Virage_90left() {
    id = VIRAGE_90LEFT;
    pos = 0;
    spritePath = "trackPieces/versionPNG/Virage90Left.png";
    spriteRotationOffset = 42.5f;
    stepAngleDeg = -45.0f / 4;
    float lengthAngleVirage = lenghtForStep(stepAngleDeg, getTurnRadius());
    angles = { stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg, stepAngleDeg };
    lengths = { lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage, lengthAngleVirage , lengthAngleVirage, lengthAngleVirage, lengthAngleVirage };
    
}

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
    lengths = { 10 };
}


// Pit
Pit::Pit() {
    id = PIT;
    pos = 0;
    float step = 10.0f; 
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

void Track::generatePitLane(int startIndex, int endIndex)
{
    pitStartIndex = startIndex;
    pitEndIndex = endIndex;

    if (pitStartIndex < 0 || pitEndIndex < 0) return;
    if (pitEndIndex >= (int)centerLine.size()) return;

    pitLane.centerLine.clear();
    pitLane.edges.left.clear();
    pitLane.edges.right.clear();
    pitLane.entryCurve.clear();
    pitLane.exitCurve.clear();
    pitLane.entryCurveEdges.left.clear();
    pitLane.entryCurveEdges.right.clear();
    pitLane.exitCurveEdges.left.clear();
    pitLane.exitCurveEdges.right.clear();

    float pitOffset = trackWidth * 2.5f; 
    float halfWidth = trackWidth * 0.5f; 
    int   curveSteps = 24;               

    int midIndex = (pitStartIndex + pitEndIndex) / 2;
    QVector2D midDir = (centerLine[midIndex + 1] - centerLine[midIndex]).normalized();
    QVector2D pitNormal = perpendicular(midDir); 

    int pitSize = pitEndIndex - pitStartIndex + 1;
    int pitRatio = pitSize / 3; 

    for (int i = (pitStartIndex + pitRatio); i <= (pitEndIndex - pitRatio); i++) {
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

        QVector2D pitPoint = centerLine[i] - pitNormal * pitOffset;
        pitLane.centerLine.push_back(pitPoint);

        pitLane.edges.left.push_back(pitPoint + perpendicular(dir) * halfWidth);
        pitLane.edges.right.push_back(pitPoint - perpendicular(dir) * halfWidth);
    }

    if (pitLane.centerLine.size() < 2) return;

    QVector2D entryStartDir = (centerLine[pitStartIndex + 1] - centerLine[pitStartIndex]).normalized();
    QVector2D entryStart = centerLine[pitStartIndex]; 

    QVector2D entryEnd = pitLane.centerLine.front();
    QVector2D entryEndDir = (pitLane.centerLine[1] - pitLane.centerLine[0]).normalized();

    float cpDist = (entryEnd - entryStart).length() * 0.5f;
    QVector2D cp1 = entryStart + entryStartDir * cpDist; 
    QVector2D cp2 = entryEnd - entryEndDir * cpDist; 

    for (int i = 0; i <= curveSteps; i++) {
        float t = (float)i / curveSteps;
        float t2 = t * t, t3 = t2 * t;
        float u = 1.0f - t;
        float u2 = u * u, u3 = u2 * u;
        QVector2D point = entryStart * u3
            + cp1 * (3 * u2 * t)
            + cp2 * (3 * u * t2)
            + entryEnd * t3;
        pitLane.entryCurve.push_back(point);
    }

    pitLane.entryCurve.front() = entryStart;
    pitLane.entryCurve.back() = entryEnd;

    for (size_t i = 0; i < pitLane.entryCurve.size(); i++) {
        QVector2D dir;
        if (i == 0)
            dir = (pitLane.entryCurve[1] - pitLane.entryCurve[0]).normalized();
        else if (i == pitLane.entryCurve.size() - 1)
            dir = (pitLane.entryCurve[i] - pitLane.entryCurve[i - 1]).normalized();
        else

            dir = ((pitLane.entryCurve[i] - pitLane.entryCurve[i - 1]) +
                (pitLane.entryCurve[i + 1] - pitLane.entryCurve[i])).normalized();
        QVector2D n = perpendicular(dir);
        pitLane.entryCurveEdges.left.push_back(pitLane.entryCurve[i] + n * halfWidth);
        pitLane.entryCurveEdges.right.push_back(pitLane.entryCurve[i] - n * halfWidth);
    }

    pitLane.entryCurveEdges.left.back() = pitLane.edges.left.front();
    pitLane.entryCurveEdges.right.back() = pitLane.edges.right.front();

    QVector2D exitStart = pitLane.centerLine.back();
    QVector2D exitStartDir = (pitLane.centerLine.back() - pitLane.centerLine[pitLane.centerLine.size() - 2]).normalized();

    QVector2D exitEndDir = (centerLine[pitEndIndex] - centerLine[pitEndIndex - 1]).normalized();
    QVector2D exitEnd = centerLine[pitEndIndex]; 

    float cpDistExit = (exitEnd - exitStart).length() * 0.5f;
    QVector2D cp3 = exitStart + exitStartDir * cpDistExit; 
    QVector2D cp4 = exitEnd - exitEndDir * cpDistExit;

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

    pitLane.exitCurve.front() = exitStart;
    pitLane.exitCurve.back() = exitEnd;

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

    pitLane.exitCurveEdges.left.front() = pitLane.edges.left.back();
    pitLane.exitCurveEdges.right.front() = pitLane.edges.right.back();
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

        default:
            qDebug() << "Invalid decor type: " << decorType;
            break;
    }
}

void Track::autoPlaceDecorsForSegment(int segmentIndex)
{
    if (segmentIndex < 0 || segmentIndex >= (int)trackSegments.size()) return;
    if (centerLine.empty()) return;

    const TrackSegment& seg = trackSegments[segmentIndex];
    if (seg.type == TrackSegmentType::PIT_TRACK) return;

    const float grandstandSpacing = 150.0f;
    const float treeSpacing = 80.0f;
    const float sideOffset = trackWidth * 1.5f; 
    const int   steps = 20; 

    int segStart = (segmentIndex == 0) ? 0 : segmentIndex * steps;
    int segEnd = segStart + steps;
    if (segEnd >= (int)centerLine.size()) segEnd = (int)centerLine.size() - 1;
    if (segStart >= segEnd) return;

    auto faceNearestCenterLine = [&](QVector2D pos) -> float {
        float minDist = 1e9f;
        int   nearest = segStart;

        for (int ci = segStart; ci <= segEnd; ci++) {
            float d = (centerLine[ci] - pos).length();
            if (d < minDist) { minDist = d; nearest = ci; }
        }
        QVector2D toCenter = (centerLine[nearest] - pos).normalized();
        return qRadiansToDegrees(std::atan2(toCenter.y(), toCenter.x())) + 90.0f;
        };

    auto spawnGrandstand = [&](QVector2D pos, int variant) {
        Grandstand* gs = new Grandstand(pos, 0.0f);
        gs->selectModel(variant % 2);
        gs->setAngle(faceNearestCenterLine(pos));
        gs->setAutoPlaced(true);
        gs->setSegmentIndex(segmentIndex);
        decors.push_back(gs);
        };

    auto spawnTree = [&](QVector2D pos, int variant) {

        for (int ci = 0; ci < (int)centerLine.size(); ci++) {
            if (ci >= segStart && ci <= segEnd) continue; 
            if ((centerLine[ci] - pos).length() < trackWidth * 1.5f) return;
        }
        TREES* tree = new TREES(pos, 0.0f);
        tree->selectModel(variant % 2);
        tree->setAngle(faceNearestCenterLine(pos));
        tree->setAutoPlaced(true);
        tree->setSegmentIndex(segmentIndex);
        decors.push_back(tree);
        };

    auto spawnGarage = [&](QVector2D pos, int variant) {

        for (int ci = 0; ci < (int)centerLine.size(); ci++) {
            if (ci >= segStart && ci <= segEnd) continue; 
            if ((centerLine[ci] - pos).length() < trackWidth * 1.5f) return;
        }
        Garage* garage = new Garage(pos, 0.0f); 
        garage->selectModel(1);
        garage->setAngle(faceNearestCenterLine(pos));
        garage->setAutoPlaced(true);
        garage->setSegmentIndex(segmentIndex);
        decors.push_back(garage);
        };


    float gsAccum = 0.0f, treeAccum = 0.0f;
    bool  gsFirst = true, treeFirst = true;

    for (int i = segStart + 1; i <= segEnd; i++) {
        float stepLen = (centerLine[i] - centerLine[i - 1]).length();
        gsAccum += stepLen;
        treeAccum += stepLen;

        QVector2D tangent = (centerLine[i] - centerLine[i - 1]).normalized();
        QVector2D leftNormal(-tangent.y(), tangent.x());
        QVector2D rightNormal(tangent.y(), -tangent.x());

        bool placeGS = gsFirst || (gsAccum >= grandstandSpacing);
        if (placeGS) {
            gsFirst = false; gsAccum = 0.0f;
            spawnGrandstand(centerLine[i] + leftNormal * sideOffset, i);
        }

        bool placeTree = treeFirst || (treeAccum >= treeSpacing);
        if (placeTree) {
            treeFirst = false; treeAccum = 0.0f;
            spawnTree(centerLine[i] + rightNormal * sideOffset, i);
            spawnTree(centerLine[i] + leftNormal * sideOffset * 1.3f, i + 1);
        }

    }

}

void Track::removeAutoDecorsForSegment(int segmentIndex)
{
    decors.erase(
        std::remove_if(decors.begin(), decors.end(),
            [segmentIndex](DecorPieces* d) -> bool {
                if (d && d->getInfo().autoPlaced &&
                    d->getInfo().segmentIndex == segmentIndex)
                {
                    delete d;
                    return true;
                }
                return false;
            }),
        decors.end());
}

void Track::autoPlaceAllDecors()
{
    for (int i = 0; i < (int)trackSegments.size(); i++)
        autoPlaceDecorsForSegment(i);
}

void Track::removeAllAutoDecors()
{
    decors.erase(
        std::remove_if(decors.begin(), decors.end(),
            [](DecorPieces* d) -> bool {
                if (d && d->getInfo().autoPlaced) { delete d; return true; }
                return false;
            }),
        decors.end());
}

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
    info.width = 1.0f;
    info.length = 1.0f; 
	info.modelType = GRANDSTAND_INDEX;
	info.modelIndex = 0;

    modelList = {
        "/3dModels/dae/grandStand.dae",
        "/3dModels/dae/grandStandCovered.dae",
        "/3dModels/dae/grandStandAwning.dae",
        "/3dModels/dae/grandStandRound.dae",
        "/3dModels/dae/grandStandCoveredRound.dae"
    };
    info.modelPath = modelList[0]; 
}

void Grandstand::selectModel(int modelNum)
{

    if (modelNum >= 0 && modelNum < (int)modelList.size()) {
        info.modelPath = modelList[modelNum];
		info.modelIndex = modelNum;
    }
    else {
        qDebug() << "Invalid model number for grandstand: " << modelNum;
        info.modelPath = modelList[0]; 
        info.modelIndex = 0; 
    }
    switch (info.modelIndex) {
        case 0: 
            info.width = 1.0f; 
			info.height = 0.9f; 
            info.length = 1.0f; 
            break;
        case 1: 
            info.width = 1.0f; 
            info.height = 1.19f; 
            info.length = 1.02f; 
            break;
        case 2: 
            info.width = 1.0f; 
            info.height = 1.39f; 
            info.length = 1.0f; 
            break;
        case 3: 
            info.width = 1.64f; 
            info.height = 0.9f; 
            info.length = 1.64f; 
            break;
        case 4:
            info.width = 1.64f; 
            info.height = 1.19f; 
            info.length = 1.64f; 
            break;
        default:
			info.width = 1.0f; 
			info.height = 0.9f; 
			info.length = 1.0f; 
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
	info.modelIndex = 0; 
    

    modelList = {
        "/3dModels/dae/pitsGarage.dae",
        "/3dModels/dae/pitsGarageClosed.dae",
        "/3dModels/dae/pitsGarageCorner.dae"
    };
    info.modelPath = modelList[0]; 
}

void Garage::selectModel(int modelNum)
{
 
    if (modelNum >= 0 && modelNum < (int)modelList.size()) {
        info.modelPath = modelList[modelNum];
		info.modelIndex = modelNum;
    }
    else {
        qDebug() << "Invalid model number for grandstand: " << modelNum;
        info.modelPath = modelList[0]; 
        info.modelIndex = 0;
    }

    switch (info.modelIndex) {
    case 0: 
        info.width = 1.0f; 
        info.height = 0.7f;
        info.length = 1.1f;
        break;
    case 1: 
        info.width = 1.0f; 
        info.height = 1.7f;
        info.length = 1.1f;
        break;
    case 2: 
        info.width = 1.05f;
        info.height = 1.7f;
        info.length = 1.0f;
        break;
    default:
        info.width = 1.0f; 
        info.height = 0.7f;
        info.length = 1.1f; 
        break;
    }

}
TREES::TREES(QVector2D p, float a)
{
    info.pos = p;
    info.angle = a;
    info.width = 0.25f; 
    info.length = 0.29f; 
	info.modelType = TREES_INDEX;
	info.modelIndex = 0; 

    modelList = {
        "/3dModels/dae/treeSmall.dae",
        "/3dModels/dae/treeLarge.dae"
    };
    info.modelPath = modelList[0]; 
}

void TREES::randomModel()
{
    info.modelIndex = rand() % modelList.size();
	selectModel(info.modelIndex);
}

void TREES::selectModel(int modelNum)
{
   
    if (modelNum >= 0 && modelNum < (int)modelList.size()) {
        info.modelPath = modelList[modelNum];
		info.modelIndex = modelNum;
    }
    else {
        qDebug() << "Invalid model number for grandstand: " << modelNum;
        info.modelPath = modelList[0]; 
		info.modelIndex = 0;
    }
    if (info.modelIndex == 0) {
        info.width = 0.25f;
        info.length = 0.29f; 
		info.height = 1.07f; 
    }
    else {
        info.width = 0.36f;
        info.length = 0.41f;
		info.height = 1.51f; 
    }

}
NoSpecificDecor::NoSpecificDecor(QVector2D p, float a)
{
    info.pos = p;
    info.angle = a;
    info.width = 1.0f; 
    info.length = 1.0f;
	info.modelType = NOSPECIFICDECOR_INDEX;
	info.modelIndex = 0; 

    modelList = {
        "decorPieces/NonSpecificModel1.obj",
        "decorPieces/NonSpecificModel2.obj",
        "decorPieces/NonSpecificModel3.obj"
    };
    info.modelPath = modelList[0]; 
void NoSpecificDecor::selectModel(int modelNum)
{
    if (modelNum >= 0 && modelNum < (int)modelList.size()) {
        info.modelPath = modelList[modelNum];
        info.modelIndex = modelNum;
    }
    else {
        qDebug() << "Invalid model number for non-specific decor: " << modelNum;
        info.modelPath = modelList[0]; 
        info.modelIndex = 0; 
    }
   
    info.width = 1.0f; 
    info.length = 1.0f;
    info.height = 1.0f; 
}

Track::Track()
    : startAngle(0)
    , currentAngle(0)
    , currentPos(0, 0)
{
   
    centerLine.push_back(currentPos);

	trackWidth = 40.0f;
    defaultMapList(); 
}

Track::Track(std::vector<int> listPieces)
    : piecesIntList(listPieces)
    , startAngle(0)
    , currentAngle(0)
    , currentPos(0, 0)
    
{
   
    centerLine.push_back(currentPos);
    trackWidth = 40.0f;
   
    for (int pieceId : piecesIntList) {
        calculAngLen(pieceId);
    }

    calculateTrackEdges();
}

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
    buildFromSegments();
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
    buildFromSegments(); 
}

void Track::removeLastSegment()
{
    if (trackSegments.empty()) return;
    trackSegments.pop_back();
    
    buildFromSegments(); 
}

void Track::addPitSegment()
{
    TrackSegment seg;
    seg.type = TrackSegmentType::PIT_TRACK;
    seg.start = QVector2D(0, 0);
    seg.end = QVector2D(400, 0);
    seg.cp1 = seg.start;
    seg.cp2 = seg.end;

    if (!trackSegments.empty()) {
       
        QVector2D offset = seg.end - trackSegments.front().start;
        for (auto& s : trackSegments) {
            s.start += offset;
            s.end += offset;
            s.cp1 += offset;
            s.cp2 += offset;
        }
    }

    trackSegments.insert(trackSegments.begin(), seg);  
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

        if (seg.type == TrackSegmentType::PIT_TRACK) {
            int segEndIndex = centerLine.size() - 1;
            generatePitLane(segStartIndex, segEndIndex);
        }

        else {
			createCheckpointAtSegment(); 
        }
    }
   
    if (!trackSegments.empty() && centerLine.size() >= 2) {
        QVector2D dir = (centerLine[1] - centerLine[0]).normalized();
        startAngle = qRadiansToDegrees(atan2f(dir.y(), dir.x()));
    }
    
}
void Track::setCurrentChoixMapData(QString mapName)
{
        for (const auto& mapData : choixMapList) {
            if (mapData.mapName == mapName) {
                currentChoixMapData = mapData;
                break;
            }
        }
}

void Track::createCheckpointAtSegment()
{
    if (centerLine.empty()) return;
    if (trackEdges.left.empty()) return;

    if (checkpoints.empty()) {

        
        CheckpointData cp;
        cp.left = trackEdges.left[0];
        cp.right = trackEdges.right[0];
        cp.forward = (centerLine[1] - centerLine[0]).normalized();
        cp.centerLineIndex = 0;
        cp.triggered = false;

        checkpoints.push_back(cp);
    }

    int clIndex = centerLine.size() - 1;

    QVector2D forward;
    if (clIndex > 0)
        forward = (centerLine[clIndex] - centerLine[clIndex - 1]).normalized();
    else
        forward = QVector2D(1, 0);

    CheckpointData cp;
    cp.left = trackEdges.left[clIndex];
    cp.right = trackEdges.right[clIndex];
    cp.forward = forward; 
    cp.centerLineIndex = clIndex;
    cp.triggered = false;

    checkpoints.push_back(cp);
}

bool Track::isBetweenPoints(const QVector2D& carPos,
    const QVector2D& pointA,
    const QVector2D& pointB,
    float threshold) const
{
   
    QVector2D AB = pointB - pointA;
    QVector2D AP = carPos - pointA;

    float ab2 = QVector2D::dotProduct(AB, AB);
    if (ab2 == 0.0f) return false;

    float t = QVector2D::dotProduct(AP, AB) / ab2;

    if (t < 0.0f || t > 1.0f) return false;

    QVector2D closest = pointA + AB * t;
    float dist = (carPos - closest).length();

    return dist <= threshold;
}

int Track::isCarBetweenCheckpoints(const QVector2D& point) const
{
    for (int i = 0; i < checkpoints.size(); i++)
       
        if (isBetweenPoints(point, checkpoints[i].left, checkpoints[i].right)) {
            return i;
        }
    return -1;
}


void Track::calculAngLen(int index)
{
    TrackPieces* piece = nullptr;

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

    std::vector<float> angles = piece->getAngles();
    std::vector<float> lengths = piece->getLengths();

    piece->setStartIndex(centerLine.size() - 1);

    for (size_t i = 0; i < angles.size() && i < lengths.size(); i++) {
        currentAngle += angles[i];
        currentPos = move(currentPos, currentAngle, lengths[i]);
        centerLine.push_back(currentPos);
    }

    piece->setEndIndex(centerLine.size() - 1);
    

    if (piece->getId() == PIT) {
        generatePitLane(piece->getStartIndex(), piece->getEndIndex()); 
		
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
          
            dir = (centerLine[1] - centerLine[0]).normalized();
        }
        else if (i == centerLine.size() - 1) {
           
            dir = (centerLine[i] - centerLine[i - 1]).normalized();
        }
        else {
           
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

void Track::closeTrack()
{
    if (isClosed()) return;
    if (centerLine.size() < 2) return;

    QVector2D start = centerLine.front();
    QVector2D end = centerLine.back();

    QVector2D startTangent = (centerLine[1] - centerLine[0]).normalized();
    QVector2D endTangent = (centerLine.back() -
        centerLine[centerLine.size() - 2]).normalized();

    float dist = (end - start).length();

    QVector2D cp1 = end + endTangent * (dist * 0.4f);
    QVector2D cp2 = start - startTangent * (dist * 0.4f);

    int steps = 20;
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / steps;
        float u = 1.0f - t;

        QVector2D point = end * (u * u * u)
            + cp1 * (3 * u * u * t)
            + cp2 * (3 * u * t * t)
            + start * (t * t * t);

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

    file << "TRACK_V1\n";

    file << "TRACK_WIDTH " << trackWidth << "\n";

    file << "START_ANGLE " << startAngle << "\n";

    file << "PIECES " << piecesIntList.size() << "\n";
    for (int pieceId : piecesIntList) {
        file << pieceId << "\n";
    }
	
    file << "TRACK_SEGMENTS " << trackSegments.size() << "\n";
    for (const auto& s : trackSegments) {
        file << (int)s.type << " "
            << s.start.x() << " " << s.start.y() << " "
            << s.end.x() << " " << s.end.y() << " "
            << s.cp1.x() << " " << s.cp1.y() << " "
            << s.cp2.x() << " " << s.cp2.y() << "\n";
    }
	
	file << "DECORS " << decors.size() << "\n";
	for (const auto& decor : decors) {
		file << decor->getInfo().modelType << " " << decor->getInfo().modelIndex << " " << decor->getInfo().pos.x() << " " << decor->getInfo().pos.y() << " " << decor->getInfo().angle << "\n";
	}

    file << "BEZIER_CURVES " << bezierCurves.size() << "\n";
    for (const auto& c : bezierCurves) {
        file << c.p0.x() << " " << c.p0.y() << " "
            << c.p1.x() << " " << c.p1.y() << " "
            << c.p2.x() << " " << c.p2.y() << " "
            << c.p3.x() << " " << c.p3.y() << "\n";
    }

    file << "CENTERLINE " << centerLine.size() << "\n";
    for (const auto& point : centerLine) {
        file << point.x() << " " << point.y() << "\n";
    }

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
                   
                    default:
                        qDebug() << "Invalid decor type in file: " << decorType;
                        continue; 
                }
                if (decor) {
                    decor->selectModel(decorIndex); 
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
          
            int count;
            iss >> count;
            for (int i = 0; i < count; i++) {
                std::getline(file, line);
            }
        }
    }

    file.close();

	
    if (loadedPiecesInt.empty() && trackSegments.empty()) {
        std::cerr << "No pieces or segments found in file" << std::endl;
        return false;
    }

    piecesIntList = loadedPiecesInt;
    pieces = loadedPieces;
	decors = loadedDecors;
	
    trackWidth = loadedTrackWidth;
    
    currentAngle = loadedStartAngle;
    currentPos = QVector2D(0, 0);

    if (!piecesIntList.empty()) {
        
        startAngle = loadedStartAngle;
        centerLine.clear();
        centerLine.push_back(currentPos);
        for (int pieceId : piecesIntList)
            calculAngLen(pieceId);
        calculateTrackEdges();
        closeTrack();
    }
    else if (!trackSegments.empty()) {
       
        closeTrack();
    }

    if (!trackSegments.empty() && centerLine.size() >= 2) {
        QVector2D dir = (centerLine[1] - centerLine[0]).normalized();
        startAngle = qRadiansToDegrees(atan2f(dir.y(), dir.x()));
    }

    std::cout << "Track loaded successfully from: " << filename
              << "  startAngle=" << startAngle << std::endl;
    return true;
}

bool Track::playTrack(QString name)
{
    if (name == "nate") {
        setCurrentChoixMapData("nate");
        loadFromFile((getCurrentChoixMapData().mapFilePath).toStdString());

        return true;
	}else if (name == "nate2") {
        
		setCurrentChoixMapData("nate2");
        loadFromFile((getCurrentChoixMapData().mapFilePath).toStdString());
        return true;
	}else if (name == "NATE3") {
        setCurrentChoixMapData("NATE3");
        loadFromFile((getCurrentChoixMapData().mapFilePath).toStdString());
		
        return true;
	}

    return false;
}


void Track::defaultMapList()
{
    choixMapList.clear();

    // --- Track 1 ---
    ChoixMapData map1;
    map1.mapIndex = 0;
    map1.mapName = "nate";
    map1.mapFilePath = "tracks/nate.trk";
    map1.mapThumbnailPath = "thumbnails/track1.png";
    map1.skyboxFilePath = "/images/skybox/space/cubemap1";

    map1.groundData.width = 2000.0f;
    map1.groundData.height = 2000.0f;
    map1.groundData.ambientColor = QColor(0, 0, 0);
    
	map1.trackData.trackTexturePath = "/images/rainbow.png";
	map1.trackData.trackColor = QColor(241, 242, 246);
    map1.trackData.kerbData.width = 5.0f;
    map1.trackData.kerbData.height = 0.05f;
    map1.trackData.kerbData.color1 = QColor(220, 30, 30);
    map1.trackData.kerbData.color2 = QColor(Qt::white);
    map1.trackData.ambientColor = QColor(80, 80, 80);

    map1.pitData.pitColor = QColor(241, 242, 246);
    map1.pitData.pitTexturePath = "/images/rainbow.jpg";
    map1.pitData.ambientColor = QColor(80, 80, 80);
    map1.pitData.kerbData.width = 5.0f;
    map1.pitData.kerbData.height = 2.0f;
    map1.pitData.kerbData.color1 = QColor(255, 255, 0);
    map1.pitData.kerbData.color2 = QColor(0, 0, 0);

    choixMapList.push_back(map1);

    // --- Track 2 ---
    ChoixMapData map2;
    map2.mapIndex = 1;
    map2.mapName = "nate2";
    map2.mapFilePath = "tracks/nate2.trk";
    map2.mapThumbnailPath = "thumbnails/track2.png";
    map2.skyboxFilePath = "/images/skybox/sky/cubemap1";

    map2.groundData.width = 5000.0f;
    map2.groundData.height = 5000.0f;
    map2.groundData.texturePath = "/images/Cartoon_green_texture_grass.jpg";
	map2.groundData.ambientColor = QColor(100, 150, 100); 

    map2.trackData.trackTexturePath = "/images/road_texture.jpg";
    map2.trackData.trackColor = QColor(241, 242, 246);
    map2.trackData.kerbData.width = 5.0f;
    map2.trackData.kerbData.height = 0.05f;
    map2.trackData.kerbData.color1 = QColor(220, 30, 30);
    map2.trackData.kerbData.color2 = QColor(Qt::white);
    map2.trackData.ambientColor = QColor(80, 80, 80);

    map2.pitData.pitColor = QColor(241, 242, 246);
    map2.pitData.pitTexturePath = "/images/road_texture.jpg";
    map2.pitData.ambientColor = QColor(80, 80, 80);
    map2.pitData.kerbData.width = 5.0f;
    map2.pitData.kerbData.height = 2.0f;
    map2.pitData.kerbData.color1 = QColor(255, 255, 0);
    map2.pitData.kerbData.color2 = QColor(0, 0, 0);
    choixMapList.push_back(map2);

    // --- Track 3 ---
    ChoixMapData map3;
  
    map3.mapIndex = 2;
    map3.mapName = "NATE3";
    map3.mapFilePath = "tracks/nate3.trk";
    map3.mapThumbnailPath = "thumbnails/track3.png";
    map3.skyboxFilePath = "/images/skybox/sky/cubemap1";

    map3.groundData.width = 4000.0f;
    map3.groundData.height = 4000.0f;
    map3.groundData.texturePath = "/images/water2.jpg";
    map3.groundData.ambientColor = QColor(153, 255, 255);

	map3.trackData.trackTexturePath = "/images/sand.jpg";
    map3.trackData.trackColor = QColor(139, 69, 19); 
    map3.trackData.kerbData.width = 5.0f;
    map3.trackData.kerbData.height = 0.05f;
    map3.trackData.kerbData.color1 = QColor(222, 184, 135); 
    map3.trackData.kerbData.color2 = QColor(Qt::white);
    map3.trackData.ambientColor = QColor(250 / 2, 232 /2 , 180/ 2);

    map3.pitData.pitColor = QColor(241, 242, 246);
    map3.pitData.pitTexturePath = "/images/sand.jpg";
    map3.pitData.ambientColor = QColor(250 / 2, 232 / 2, 180 / 2);
    map3.pitData.kerbData.width = 5.0f;
    map3.pitData.kerbData.height = 2.0f;
    map3.pitData.kerbData.color1 = QColor(255, 255, 0);
    map3.pitData.kerbData.color2 = QColor(0, 0, 0);

    choixMapList.push_back(map3);
}


#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , currentAngle(0)
    , currentPos(0, 0)
{
    setWindowTitle("Track Builder");
    resize(1000, 700);

    // Central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Track viewer
    trackViewer = new TrackViewer(this);
    mainLayout->addWidget(trackViewer);

    // Controls
    QHBoxLayout* controlsLayout = new QHBoxLayout();

    QLabel* label = new QLabel("Segment Type:", this);
    controlsLayout->addWidget(label);

    segmentTypeCombo = new QComboBox(this);
    segmentTypeCombo->addItem("^");
    segmentTypeCombo->addItem("45 <-");
    segmentTypeCombo->addItem("45 ->");
    segmentTypeCombo->addItem("90 <-");
    segmentTypeCombo->addItem("90 ->");
    controlsLayout->addWidget(segmentTypeCombo);

    addButton = new QPushButton("Add Segment", this);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddSegment);
    controlsLayout->addWidget(addButton);

    clearButton = new QPushButton("Clear Track", this);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearTrack);
    controlsLayout->addWidget(clearButton);

    controlsLayout->addStretch();
    mainLayout->addLayout(controlsLayout);

    // Initialize track
    track.push_back(currentPos);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onAddSegment()
{
    std::vector<float> angles;
    std::vector<float> lengths;

    QString type = segmentTypeCombo->currentText();

    if (type == "^") {
        angles = { 0, 0, 0, 0, 0 };
        lengths = { 20, 20, 20, 20, 20 };
    }
    else if (type == "45 ->") {
        angles = { 0, 45.0f / 2, 0, 45.0f / 2, 0 };
        lengths = { 20, 10, 10, 10, 20 };
    }
    else if (type == "45 <-") {
        angles = { 0, -45.0f / 2, 0, -45.0f / 2, 0 };
        lengths = { 20, 10, 10, 10, 20 };
    }
    else if (type == "90 ->") {
        angles = { 0, 45, 0, 45, 0 };
        lengths = { 20, 10, 10, 10, 20 };
    }
    else if (type == "90 <-") {
        angles = { 0, -45, 0, -45, 0 };
        lengths = { 20, 10, 10, 10, 20 };
    }

    // Add segment
    for (size_t i = 0; i < angles.size(); i++) {
        currentAngle += angles[i];
        currentPos = currentPos.move(currentAngle, lengths[i]);
        track.push_back(currentPos);
    }

    // Update display
    trackInfo displayTrack;
    displayTrack.start = track[0];
    displayTrack.centerLine = track;
    displayTrack.trackEdges = map.calculateTrackEdges(track, 40);

    trackViewer->updateTrack(displayTrack);
}

void MainWindow::onClearTrack()
{
    track.clear();
    currentPos = Vec2(0, 0);
    currentAngle = 0;
    track.push_back(currentPos);

    trackInfo emptyTrack;
    emptyTrack.start = currentPos;
    trackViewer->updateTrack(emptyTrack);
}
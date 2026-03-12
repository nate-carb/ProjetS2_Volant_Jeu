#include "mainWindowCreator.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>


MainWindowCreator::MainWindowCreator(QWidget* parent)
    : QMainWindow(parent)

{
    setWindowTitle("Track Creator");
    resize(1200, 800);

    // Central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    // Track viewer (left side)
    trackCreator = new TrackCreator(this);
    mainLayout->addWidget(trackCreator, 3);  // Takes 3/4 of space

    // Control panel (right side)
    QVBoxLayout* controlLayout = new QVBoxLayout();
    mainLayout->addLayout(controlLayout, 1);  // Takes 1/4 of space

    // Title
    QLabel* titleLabel = new QLabel("Track Pieces", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    controlLayout->addWidget(titleLabel);

    // Track pieces buttons
    QGroupBox* piecesGroup = new QGroupBox("Add Pieces", this);
    QVBoxLayout* piecesLayout = new QVBoxLayout(piecesGroup);
    createPieceButtons(piecesLayout);
    controlLayout->addWidget(piecesGroup);

    // Track controls
    QGroupBox* trackControlsGroup = new QGroupBox("Track Controls", this);
    QVBoxLayout* trackControlsLayout = new QVBoxLayout(trackControlsGroup);

    QPushButton* undoBtn = new QPushButton("Undo Last Piece", this);
    connect(undoBtn, &QPushButton::clicked, this, &MainWindowCreator::onUndo);
    trackControlsLayout->addWidget(undoBtn);

    QPushButton* clearBtn = new QPushButton("Clear Track", this);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindowCreator::onClear);
    trackControlsLayout->addWidget(clearBtn);

    QPushButton* saveBtn = new QPushButton("Save Track", this);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindowCreator::onSave);
    trackControlsLayout->addWidget(saveBtn);

    QPushButton* loadBtn = new QPushButton("Load Track", this);
    connect(loadBtn, &QPushButton::clicked, this, &MainWindowCreator::onLoad);
    trackControlsLayout->addWidget(loadBtn);


    controlLayout->addWidget(trackControlsGroup);
    /*
    // Add 3D View button
    QPushButton* view3DBtn = new QPushButton("View in 3D", this);
    view3DBtn->setStyleSheet("QPushButton { background-color: #9C27B0; color: white; padding: 8px; }");
    connect(view3DBtn, &QPushButton::clicked, this, &MainWindowCreator::onView3D);
    controlLayout->addWidget(view3DBtn);
    */
    // Status label
    statusLabel = new QLabel("Pieces: 0", this);
    controlLayout->addWidget(statusLabel);

    controlLayout->addStretch();

    // Connect track updates
    connect(trackCreator, &TrackCreator::trackUpdated, this, [this](const Track& track) {
        statusLabel->setText(QString("Pieces: %1").arg(track.getCenterLine().size()));
        });

}

MainWindowCreator::~MainWindowCreator()
{
}

void MainWindowCreator::createPieceButtons(QVBoxLayout* layout)
{
	// ── Track Segments dropdown ──────────────────────────────────────────
    QLabel* segLabel = new QLabel("── Track Segments ──", this);
    segLabel->setAlignment(Qt::AlignCenter);
    segLabel->setStyleSheet("color: #aaa; font-size: 11px;");
    layout->addWidget(segLabel);

    QPushButton* addCurveBtn = new QPushButton("Add Curve", this);
    addCurveBtn->setStyleSheet(
        "QPushButton { background-color: #E91E63; color: white; padding: 8px; }");
    connect(addCurveBtn, &QPushButton::clicked, [this]() {
		trackCreator->addCurveSegment();   
        });
    layout->addWidget(addCurveBtn);

    QPushButton* addStraightBtn = new QPushButton("Add Straight", this);
    addStraightBtn->setStyleSheet(
        "QPushButton { background-color: #9C27B0; color: white; padding: 8px; }");
    connect(addStraightBtn, &QPushButton::clicked, [this]() {
        trackCreator->addStraightSegment();
        
        });
    layout->addWidget(addStraightBtn);

    QPushButton* removeLastBtn = new QPushButton("Remove Last", this);
    removeLastBtn->setStyleSheet(
        "QPushButton { background-color: #F44336; color: white; padding: 8px; }");
    connect(removeLastBtn, &QPushButton::clicked, [this]() {
		trackCreator->removeLastSegment();
        });
    layout->addWidget(removeLastBtn);
	/* OLD BUTTONS for track pieces, replaced by the above ones for better user experience
    // Straight
    QPushButton* straightBtn = new QPushButton("Straight", this);
    straightBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px; }");
    connect(straightBtn, &QPushButton::clicked, [this]() { onAddPiece(STRAIGHT); });
    layout->addWidget(straightBtn);

    // 45 Left
    QPushButton* left45Btn = new QPushButton("45� Left Turn", this);
    left45Btn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px; }");
    connect(left45Btn, &QPushButton::clicked, [this]() { onAddPiece(VIRAGE_45LEFT); });
    layout->addWidget(left45Btn);

    // 45 Right
    QPushButton* right45Btn = new QPushButton("45� Right Turn", this);
    right45Btn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px; }");
    connect(right45Btn, &QPushButton::clicked, [this]() { onAddPiece(VIRAGE_45RIGHT); });
    layout->addWidget(right45Btn);

    // 90 Left
    QPushButton* left90Btn = new QPushButton("90� Left Turn", this);
    left90Btn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 8px; }");
    connect(left90Btn, &QPushButton::clicked, [this]() { onAddPiece(VIRAGE_90LEFT); });
    layout->addWidget(left90Btn);

    // 90 Right
    QPushButton* right90Btn = new QPushButton("90� Right Turn", this);
    right90Btn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 8px; }");
    connect(right90Btn, &QPushButton::clicked, [this]() { onAddPiece(VIRAGE_90RIGHT); });
    layout->addWidget(right90Btn);
    */
    // PIT
    QPushButton* pitBtn = new QPushButton("Pit", this);
    pitBtn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 8px; }");
    connect(pitBtn, &QPushButton::clicked, [this]() { onAddPiece(PIT); });
    layout->addWidget(pitBtn);
    
    // ── Decor dropdown ──────────────────────────────────────────
    // Separator label
    QLabel* decorLabel = new QLabel("── Decors ──", this);
    decorLabel->setAlignment(Qt::AlignCenter);
    decorLabel->setStyleSheet("color: #aaa; font-size: 11px;");
    layout->addWidget(decorLabel);

    // Grandstand submenu
    QComboBox* grandstandCombo = new QComboBox(this);
    grandstandCombo->addItem("Grandstand (Open)");         // index 0
    grandstandCombo->addItem("Grandstand (Covered)");      // index 1
    grandstandCombo->addItem("Grandstand (Awning)");       // index 2
    grandstandCombo->addItem("Grandstand (Round)");        // index 3
    grandstandCombo->addItem("Grandstand (Covered Round)");// index 4
    grandstandCombo->setStyleSheet("padding: 4px;");
    layout->addWidget(grandstandCombo);

    QPushButton* addGrandstandBtn = new QPushButton("Add Grandstand", this);
    addGrandstandBtn->setStyleSheet("QPushButton { background-color: #9C27B0; color: white; padding: 8px; }");
    connect(addGrandstandBtn, &QPushButton::clicked, [this, grandstandCombo]() {
        onAddDecor(GRANDSTAND_INDEX, grandstandCombo->currentIndex());
        });
    layout->addWidget(addGrandstandBtn);

    // Garage submenu
    QComboBox* garageCombo = new QComboBox(this);
    garageCombo->addItem("Garage (Open)");    // index 0
    garageCombo->addItem("Garage (Closed)");  // index 1
    garageCombo->addItem("Garage (Corner)");  // index 2
    garageCombo->setStyleSheet("padding: 4px;");
    layout->addWidget(garageCombo);

    QPushButton* addGarageBtn = new QPushButton("Add Garage", this);
    addGarageBtn->setStyleSheet("QPushButton { background-color: #795548; color: white; padding: 8px; }");
    connect(addGarageBtn, &QPushButton::clicked, [this, garageCombo]() {
        onAddDecor(GARAGE_INDEX, garageCombo->currentIndex());
        });
    layout->addWidget(addGarageBtn);

    // Trees submenu
    QComboBox* treesCombo = new QComboBox(this);
    treesCombo->addItem("Tree (Small)"); // index 0
    treesCombo->addItem("Tree (Large)"); // index 1
    treesCombo->setStyleSheet("padding: 4px;");
    layout->addWidget(treesCombo);

    QPushButton* addTreeBtn = new QPushButton("Add Tree", this);
    addTreeBtn->setStyleSheet("QPushButton { background-color: #388E3C; color: white; padding: 8px; }");
    connect(addTreeBtn, &QPushButton::clicked, [this, treesCombo]() {
        onAddDecor(TREES_INDEX, treesCombo->currentIndex());
        });
    layout->addWidget(addTreeBtn);

    // ── Decor angle controls ─────────────────────────────────
    QLabel* angleLabel = new QLabel("── Decor Angle ──", this);
    angleLabel->setAlignment(Qt::AlignCenter);
    angleLabel->setStyleSheet("color: #aaa; font-size: 11px;");
    layout->addWidget(angleLabel);

    QHBoxLayout* angleLayout = new QHBoxLayout();

    QPushButton* exactAngleBtn = new QPushButton("Set Angle", this);
    exactAngleBtn->setStyleSheet(
        "QPushButton { background-color: #FF5722; color: white; padding: 6px; font-size: 10px; }");
    connect(exactAngleBtn, &QPushButton::clicked, [this]() {
        bool ok;
        double angle = QInputDialog::getDouble(this, "Set Exact Angle",
            "Angle (degrees):", 0.0, -360.0, 360.0, 1, &ok);
        if (ok) {
			trackCreator->rotateDecorExact(angle); //set selected decor angle to user input (exact)
            qDebug() << "Set exact angle:" << angle;
        }
        });
    angleLayout->addWidget(exactAngleBtn);

    QPushButton* relAngleBtn = new QPushButton("Rotate", this);
    relAngleBtn->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; padding: 6px; font-size: 10px; }");
    connect(relAngleBtn, &QPushButton::clicked, [this]() {
		trackCreator->rotateDecorRelative(5); //rotate selected decor by 5 degrees (relative)
            
        qDebug() << "Rotate by:" << 5;
        
        });
    angleLayout->addWidget(relAngleBtn);

    QPushButton* resetAngleBtn = new QPushButton("Reset", this);
    resetAngleBtn->setStyleSheet(
        "QPushButton { background-color: #607D8B; color: white; padding: 6px; font-size: 10px; }");
    connect(resetAngleBtn, &QPushButton::clicked, [this]() {
		trackCreator->rotateDecorExact(0); //reset selected decor angle to 0 degrees (exact) 
        qDebug() << "Reset angle";
        });
    angleLayout->addWidget(resetAngleBtn);

    layout->addLayout(angleLayout);

    // ── Bezier curves controls ───────────────────────────────
    QLabel* bezierLabel = new QLabel("── Bezier Curves ──", this);
    bezierLabel->setAlignment(Qt::AlignCenter);
    bezierLabel->setStyleSheet("color: #aaa; font-size: 11px;");
    layout->addWidget(bezierLabel);

    QPushButton* addBezierCurveBtn = new QPushButton("Add Bezier Curve", this);
    addBezierCurveBtn->setStyleSheet(
        "QPushButton { background-color: #00BCD4; color: white; padding: 8px; }");
    connect(addBezierCurveBtn, &QPushButton::clicked, [this]() {
        trackCreator->addBezierCurveAtCenter(); // logic in TrackCreator
        });
    layout->addWidget(addBezierCurveBtn);

    QPushButton* editCurveBtn = new QPushButton("Toggle Edit Mode", this);
    editCurveBtn->setCheckable(true);
    editCurveBtn->setStyleSheet(
        "QPushButton { background-color: #607D8B; color: white; padding: 8px; }"
        "QPushButton:checked { background-color: #00BCD4; }");
    connect(editCurveBtn, &QPushButton::toggled, [this](bool checked) {
        trackCreator->toggleBezierEditMode(checked); // logic in TrackCreator
        });
    layout->addWidget(editCurveBtn);

    // ── Close track btn ───────────────────────────────
    QPushButton* closeBtn = new QPushButton("Close Track", this);
    closeBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; padding: 8px; }");
    connect(closeBtn, &QPushButton::clicked, [this]() {
        trackCreator->closeTrack(); // use TrackCreator method not Track directly
        });
    layout->addWidget(closeBtn);
}

void MainWindowCreator::onAddPiece(int pieceType)
// Cette fonction capte les clics de souris
{
    qDebug() << "Adding piece type:" << pieceType;  // Debug output
    trackCreator->addPiece(pieceType);
}

void MainWindowCreator::onAddDecor(int decorType, int variant)
{
    qDebug() << "Adding decor type:" << decorType << "variant:" << variant;  // Debug output
    trackCreator->addDecor(decorType, variant);
}

void MainWindowCreator::onUndo()
{
    // TODO: Implement undo functionality
    // You'll need to add a method to TrackCreator to remove last piece
    QMessageBox::information(this, "Undo", "Undo functionality - to be implemented");
    // Redemande � Qt de redessiner
    update();
}

void MainWindowCreator::onClear() {
    trackCreator->clearTrack();
    statusLabel->setText("Pieces: 0");
}


void MainWindowCreator::onSave()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save Track",
        "",
        "Track Files (*.trk);;All Files (*)"
    );

    if (!filename.isEmpty()) {
        Track currentTrack = trackCreator->getCurrentTrack();

        if (currentTrack.saveToFile(filename.toStdString())) {
            QMessageBox::information(this, "Save Successful",
                "Track saved to:\n" + filename);
            statusLabel->setText("Track saved successfully");
        }
        else {
            QMessageBox::warning(this, "Save Failed",
                "Failed to save track to:\n" + filename);
        }
    }

}

void MainWindowCreator::onLoad()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Load Track",
        "",
        "Track Files (*.trk);;All Files (*)"
    );

    if (!filename.isEmpty()) {
        Track loadedTrack;

        if (loadedTrack.loadFromFile(filename.toStdString())) {
            trackCreator->loadTrack(loadedTrack);
            QMessageBox::information(this, "Load Successful",
                "Track loaded from:\n" + filename);
            statusLabel->setText(QString("Track loaded - Pieces: %1")
                .arg(loadedTrack.getPiecesList().size()));
        }
        else {
            QMessageBox::warning(this, "Load Failed",
                "Failed to load track from:\n" + filename);
        }
    }
}
/*
void MainWindowCreator::onView3D()
{
    // Save current track to temp file
    Track currentTrack = trackCreator->getCurrentTrack();
    QString tempFile = "temp_track.trk";

    if (currentTrack.saveToFile(tempFile.toStdString())) {
        // Open 3D viewer window
        Track3DViewer* viewer3D = new Track3DViewer();
        viewer3D->setWindowTitle("3D Track View");
        viewer3D->resize(1024, 768);
        viewer3D->loadTrackFile(tempFile);
        viewer3D->show();
    }

    update();
}
*/



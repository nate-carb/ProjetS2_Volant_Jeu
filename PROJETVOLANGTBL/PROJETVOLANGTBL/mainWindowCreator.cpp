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

    // Track viewer 
    trackCreator = new TrackCreator(this);
    mainLayout->addWidget(trackCreator, 3);  

    // Control panel 
    QVBoxLayout* controlLayout = new QVBoxLayout();
    mainLayout->addLayout(controlLayout, 1);  

    // Title
    QLabel* titleLabel = new QLabel("Track Pieces", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    controlLayout->addWidget(titleLabel);

    // Track 
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
    statusLabel = new QLabel("Pieces: 0", this);
    controlLayout->addWidget(statusLabel);

    controlLayout->addStretch();

    // Connect track 
    connect(trackCreator, &TrackCreator::trackUpdated, this, [this](const Track& track) {
        statusLabel->setText(QString("Pieces: %1").arg(track.getCenterLine().size()));
        });

}

MainWindowCreator::~MainWindowCreator()
{
}

void MainWindowCreator::createPieceButtons(QVBoxLayout* layout)
{
	// Track segments
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

	// Pitstop
    QPushButton* pitBtn = new QPushButton("Pit", this);
    pitBtn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 8px; }");
    connect(pitBtn, &QPushButton::clicked, [this]() {
        trackCreator->addPitSegment();
        });
    layout->addWidget(pitBtn);
 
    QLabel* decorLabel = new QLabel("── Decors ──", this);
    decorLabel->setAlignment(Qt::AlignCenter);
    decorLabel->setStyleSheet("color: #aaa; font-size: 11px;");
    layout->addWidget(decorLabel);

    // Grandstand 
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

    // Garage 
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

    // Arbres
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
			trackCreator->rotateDecorExact(angle); 
            qDebug() << "Set exact angle:" << angle;
        }
        });
    angleLayout->addWidget(exactAngleBtn);

    QPushButton* relAngleBtn = new QPushButton("Rotate", this);
    relAngleBtn->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; padding: 6px; font-size: 10px; }");
    connect(relAngleBtn, &QPushButton::clicked, [this]() {
		trackCreator->rotateDecorRelative(5); 
            
        qDebug() << "Rotate by:" << 5;
        
        });
    angleLayout->addWidget(relAngleBtn);

    QPushButton* resetAngleBtn = new QPushButton("Reset", this);
    resetAngleBtn->setStyleSheet(
        "QPushButton { background-color: #607D8B; color: white; padding: 6px; font-size: 10px; }");
    connect(resetAngleBtn, &QPushButton::clicked, [this]() {
		trackCreator->rotateDecorExact(0); 
        qDebug() << "Reset angle";
        });
    angleLayout->addWidget(resetAngleBtn);

    layout->addLayout(angleLayout);

    // Bezier
    QLabel* bezierLabel = new QLabel("── Bezier Curves ──", this);
    bezierLabel->setAlignment(Qt::AlignCenter);
    bezierLabel->setStyleSheet("color: #aaa; font-size: 11px;");
    layout->addWidget(bezierLabel);

    QPushButton* addBezierCurveBtn = new QPushButton("Add Bezier Curve", this);
    addBezierCurveBtn->setStyleSheet(
        "QPushButton { background-color: #00BCD4; color: white; padding: 8px; }");
    connect(addBezierCurveBtn, &QPushButton::clicked, [this]() {
        trackCreator->addBezierCurveAtCenter(); 
        });
    layout->addWidget(addBezierCurveBtn);

    QPushButton* editCurveBtn = new QPushButton("Toggle Edit Mode", this);
    editCurveBtn->setCheckable(true);
    editCurveBtn->setStyleSheet(
        "QPushButton { background-color: #607D8B; color: white; padding: 8px; }"
        "QPushButton:checked { background-color: #00BCD4; }");
    connect(editCurveBtn, &QPushButton::toggled, [this](bool checked) {
        trackCreator->toggleBezierEditMode(checked); 
        });
    layout->addWidget(editCurveBtn);

    // Close track 
    QPushButton* closeBtn = new QPushButton("Close Track", this);
    closeBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; padding: 8px; }");
    connect(closeBtn, &QPushButton::clicked, [this]() {
        trackCreator->closeTrack(); 
        });
    layout->addWidget(closeBtn);
    // Decors Auto
    QPushButton* autoDecorsBtn = new QPushButton("Auto Place Decors", this);
    autoDecorsBtn->setStyleSheet(
        "QPushButton { background-color: #FF6F00; color: white; padding: 8px; }");
    connect(autoDecorsBtn, &QPushButton::clicked, [this]() {
        trackCreator->autoDecors();
        });
    layout->addWidget(autoDecorsBtn);
    // Enleve Decors Auto
    QPushButton* removeAutoDecorsBtn = new QPushButton("Remove Auto Decors", this);
    removeAutoDecorsBtn->setStyleSheet(
        "QPushButton { background-color: #B71C1C; color: white; padding: 8px; }");
    connect(removeAutoDecorsBtn, &QPushButton::clicked, [this]() {
        trackCreator->removeAutoDecors();
        });
    layout->addWidget(removeAutoDecorsBtn);
}

void MainWindowCreator::onAddPiece(int pieceType)
// Capte les clics de souris
{
    qDebug() << "Adding piece type:" << pieceType;  
    trackCreator->addPiece(pieceType);
}

void MainWindowCreator::onAddDecor(int decorType, int variant)
{
    qDebug() << "Adding decor type:" << decorType << "variant:" << variant;  
    trackCreator->addDecor(decorType, variant);
}

void MainWindowCreator::onUndo()
{
    QMessageBox::information(this, "Undo", "Undo functionality - to be implemented");
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


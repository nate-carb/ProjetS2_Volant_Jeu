#include "mainWindowCreator.h"
#include <QFileDialog>
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
    // Straight
    QPushButton* straightBtn = new QPushButton("Straight", this);
    straightBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px; }");
    connect(straightBtn, &QPushButton::clicked, [this]() { onAddPiece(STRAIGHT); });
    layout->addWidget(straightBtn);

    // 45� Left
    QPushButton* left45Btn = new QPushButton("45� Left Turn", this);
    left45Btn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px; }");
    connect(left45Btn, &QPushButton::clicked, [this]() { onAddPiece(VIRAGE_45LEFT); });
    layout->addWidget(left45Btn);

    // 45� Right
    QPushButton* right45Btn = new QPushButton("45� Right Turn", this);
    right45Btn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px; }");
    connect(right45Btn, &QPushButton::clicked, [this]() { onAddPiece(VIRAGE_45RIGHT); });
    layout->addWidget(right45Btn);

    // 90� Left
    QPushButton* left90Btn = new QPushButton("90� Left Turn", this);
    left90Btn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 8px; }");
    connect(left90Btn, &QPushButton::clicked, [this]() { onAddPiece(VIRAGE_90LEFT); });
    layout->addWidget(left90Btn);

    // 90� Right
    QPushButton* right90Btn = new QPushButton("90� Right Turn", this);
    right90Btn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 8px; }");
    connect(right90Btn, &QPushButton::clicked, [this]() { onAddPiece(VIRAGE_90RIGHT); });
    layout->addWidget(right90Btn);



}

void MainWindowCreator::onAddPiece(int pieceType)
// Cette fonction capte les clics de souris
{
    qDebug() << "Adding piece type:" << pieceType;  // Debug output
    trackCreator->addPiece(pieceType);
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
<<<<<<< Updated upstream
}
=======
}
*/
>>>>>>> Stashed changes

#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <track3DViewer.h>
#include "TrackCreator.h"
#include "Track.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onAddPiece(int pieceType);
    void onUndo();
    void onClear();
    void onSave();
    void onLoad();
    void onView3D();

private:
    TrackCreator* trackCreator;
    QLabel* statusLabel;

    void createPieceButtons(QVBoxLayout* layout);
};
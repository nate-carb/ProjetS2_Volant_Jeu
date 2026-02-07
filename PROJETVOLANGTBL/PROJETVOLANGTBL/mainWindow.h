#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include "TrackViewer.h"
#include "map2d.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onAddSegment();
    void onClearTrack();

private:
    TrackViewer* trackViewer;
    QComboBox* segmentTypeCombo;
    QPushButton* addButton;
    QPushButton* clearButton;

    Map2D map;
    std::vector<Vec2> track;
    float currentAngle;
    Vec2 currentPos;
};
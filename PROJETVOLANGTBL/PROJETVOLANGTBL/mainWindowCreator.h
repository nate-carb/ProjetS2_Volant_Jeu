#pragma once
#ifndef MAINWINDOWCREATOR_H
#define MAINWINDOWCREATOR_H
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMenu>
#include <QToolButton>
#include "track3DViewer.h"
#include "TrackCreator.h"
#include "Track.h"

class MainWindowCreator : public QMainWindow
{
public:
    MainWindowCreator(QWidget* parent = nullptr);
    ~MainWindowCreator();

    //protected:
       // void paintEvent(QPaintEvent* event) override;
       // void mousePressEvent(QMouseEvent* event) override;


private slots:
    void onAddPiece(int pieceType);
    void onUndo();
    void onClear();
    void onSave();
    void onLoad();
    //void onView3D();



private:
    TrackCreator* trackCreator;
    QLabel* statusLabel;
    void createPieceButtons(QVBoxLayout* layout);
	int decorPiece; // Variable pour stocker le type de pièce de décor sélectionné
};


#endif


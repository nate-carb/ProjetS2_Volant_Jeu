#pragma once
#include <QMainWindow>
#include <trackViewer.h>
#include <qlabel.h>
#include <QHBoxLayout>

class MainWindowView : public QMainWindow
    
{
    Q_OBJECT

public:
    MainWindowView(QWidget* parent = nullptr);
    ~MainWindowView();

private:
    TrackViewer* trackViewer;
    QLabel* statusLabel;
};


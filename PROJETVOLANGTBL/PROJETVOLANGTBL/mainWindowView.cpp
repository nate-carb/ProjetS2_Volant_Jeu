#include "mainWindowView.h"
MainWindowView::MainWindowView(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Track Creator");
    resize(1200, 800);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    // Track viewer (left side)
    trackViewer = new TrackViewer(this);
    mainLayout->addWidget(trackViewer);

   
	

}

MainWindowView::~MainWindowView()
{
}

#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PROJETVOLANGTBL.h"

class PROJETVOLANGTBL : public QMainWindow
{
    Q_OBJECT

public:
    PROJETVOLANGTBL(QWidget *parent = nullptr);
    ~PROJETVOLANGTBL();

private:
    Ui::PROJETVOLANGTBLClass ui;
};


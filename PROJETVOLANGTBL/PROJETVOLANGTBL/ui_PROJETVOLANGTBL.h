/********************************************************************************
** Form generated from reading UI file 'PROJETVOLANGTBL.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROJETVOLANGTBL_H
#define UI_PROJETVOLANGTBL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PROJETVOLANGTBLClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *PROJETVOLANGTBLClass)
    {
        if (PROJETVOLANGTBLClass->objectName().isEmpty())
            PROJETVOLANGTBLClass->setObjectName("PROJETVOLANGTBLClass");
        PROJETVOLANGTBLClass->resize(600, 400);
        menuBar = new QMenuBar(PROJETVOLANGTBLClass);
        menuBar->setObjectName("menuBar");
        PROJETVOLANGTBLClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(PROJETVOLANGTBLClass);
        mainToolBar->setObjectName("mainToolBar");
        PROJETVOLANGTBLClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(PROJETVOLANGTBLClass);
        centralWidget->setObjectName("centralWidget");
        PROJETVOLANGTBLClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(PROJETVOLANGTBLClass);
        statusBar->setObjectName("statusBar");
        PROJETVOLANGTBLClass->setStatusBar(statusBar);

        retranslateUi(PROJETVOLANGTBLClass);

        QMetaObject::connectSlotsByName(PROJETVOLANGTBLClass);
    } // setupUi

    void retranslateUi(QMainWindow *PROJETVOLANGTBLClass)
    {
        PROJETVOLANGTBLClass->setWindowTitle(QCoreApplication::translate("PROJETVOLANGTBLClass", "PROJETVOLANGTBL", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PROJETVOLANGTBLClass: public Ui_PROJETVOLANGTBLClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROJETVOLANGTBL_H

#pragma once
#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include "Vehicule.h"

class DevMenu : public QDialog {
    Q_OBJECT
public:
    DevMenu(Vehicule* vehicule, QWidget* parent = nullptr);

private:
    Vehicule* m_vehicule;

    // Helper pour créer une ligne slider + valeur
    QSlider* addSlider(QVBoxLayout* layout, const QString& label,
        float min, float max, float value, float* target);

    void createDrivingGroup(QVBoxLayout* mainLayout);
    void createGearGroup(QVBoxLayout* mainLayout);
    void createBrakingGroup(QVBoxLayout* mainLayout);
};

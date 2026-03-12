#include "DevMenu.h"

DevMenu::DevMenu(Vehicule* vehicule, QWidget* parent)
    : QDialog(parent), m_vehicule(vehicule)
{
    setWindowTitle("Dev Menu - Vehicule Tuning");
    setMinimumWidth(400);
    setStyleSheet(R"(
        QDialog { background: #1a1a2e; color: #eee; }
        QGroupBox { 
            border: 1px solid #e94560; 
            border-radius: 4px; 
            margin-top: 8px;
            padding: 8px;
            font-weight: bold;
            color: #e94560;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 8px; }
        QLabel { color: #ccc; font-size: 11px; }
        QSlider::groove:horizontal { 
            height: 4px; background: #333; border-radius: 2px; 
        }
        QSlider::handle:horizontal { 
            background: #e94560; width: 14px; height: 14px;
            margin: -5px 0; border-radius: 7px; 
        }
        QSlider::sub-page:horizontal { background: #e94560; border-radius: 2px; }
        QDoubleSpinBox { 
            background: #16213e; color: #eee; 
            border: 1px solid #444; border-radius: 3px;
            padding: 2px 4px; width: 60px;
        }
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    createDrivingGroup(mainLayout);
    createGearGroup(mainLayout);
    createBrakingGroup(mainLayout);

    mainLayout->addStretch();
}

// Helper - crée label + slider + spinbox liés ensemble
QSlider* DevMenu::addSlider(QVBoxLayout* layout, const QString& label,
    float min, float max, float value, float* target)
{
    QHBoxLayout* row = new QHBoxLayout();

    QLabel* lbl = new QLabel(label);
    lbl->setMinimumWidth(160);

    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 1000);
    slider->setValue((int)((value - min) / (max - min) * 1000));

    QDoubleSpinBox* spin = new QDoubleSpinBox();
    spin->setRange(min, max);
    spin->setValue(value);
    spin->setDecimals(2);
    spin->setSingleStep((max - min) / 100.0);

    // Slider → spinbox → target
    connect(slider, &QSlider::valueChanged, [=](int v) {
        float newVal = min + (v / 1000.0f) * (max - min);
        spin->blockSignals(true);
        spin->setValue(newVal);
        spin->blockSignals(false);
        *target = newVal;
        });

    // Spinbox → slider → target
    connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double v) {
        slider->blockSignals(true);
        slider->setValue((int)((v - min) / (max - min) * 1000));
        slider->blockSignals(false);
        *target = (float)v;
        });

    row->addWidget(lbl);
    row->addWidget(slider, 1);
    row->addWidget(spin);
    layout->addLayout(row);

    return slider;
}

void DevMenu::createDrivingGroup(QVBoxLayout* mainLayout)
{
    QGroupBox* group = new QGroupBox("Conduite");
    QVBoxLayout* layout = new QVBoxLayout(group);

    addSlider(layout, "Accélération max", 10, 150, m_vehicule->maxAcceleration, &m_vehicule->maxAcceleration);
    addSlider(layout, "Turn responsiveness", 1, 20, m_vehicule->turnResponsiveness, &m_vehicule->turnResponsiveness);
    addSlider(layout, "Turn factor min", 0.0, 1.0, m_vehicule->minTurnFactor, &m_vehicule->minTurnFactor);
    addSlider(layout, "Turn factor max", 0.0, 5.0, m_vehicule->maxTurnFactor, &m_vehicule->maxTurnFactor);
    addSlider(layout, "NOS force", 10, 150, m_vehicule->nosForce, &m_vehicule->nosForce);
    addSlider(layout, "NOS drain", 1, 50, m_vehicule->nosDrain, &m_vehicule->nosDrain);

    mainLayout->addWidget(group);
}

void DevMenu::createGearGroup(QVBoxLayout* mainLayout)
{
    QGroupBox* group = new QGroupBox("Gears - Vitesse max (km/h)");
    QVBoxLayout* layout = new QVBoxLayout(group);

    addSlider(layout, "Gear 1 max", 10, 100, m_vehicule->gearMaxSpeed[1], &m_vehicule->gearMaxSpeed[1]);
    addSlider(layout, "Gear 2 max", 20, 150, m_vehicule->gearMaxSpeed[2], &m_vehicule->gearMaxSpeed[2]);
    addSlider(layout, "Gear 3 max", 40, 180, m_vehicule->gearMaxSpeed[3], &m_vehicule->gearMaxSpeed[3]);
    addSlider(layout, "Gear 4 max", 60, 210, m_vehicule->gearMaxSpeed[4], &m_vehicule->gearMaxSpeed[4]);
    addSlider(layout, "Gear 5 max", 80, 230, m_vehicule->gearMaxSpeed[5], &m_vehicule->gearMaxSpeed[5]);
    addSlider(layout, "Gear 6 max", 100, 300, m_vehicule->gearMaxSpeed[6], &m_vehicule->gearMaxSpeed[6]);

    mainLayout->addWidget(group);

    QGroupBox* group2 = new QGroupBox("Gears - Vitesse min (stall)");
    QVBoxLayout* layout2 = new QVBoxLayout(group2);

    addSlider(layout2, "Gear 2 min", 0, 80, m_vehicule->gearMinSpeed[2], &m_vehicule->gearMinSpeed[2]);
    addSlider(layout2, "Gear 3 min", 0, 100, m_vehicule->gearMinSpeed[3], &m_vehicule->gearMinSpeed[3]);
    addSlider(layout2, "Gear 4 min", 0, 140, m_vehicule->gearMinSpeed[4], &m_vehicule->gearMinSpeed[4]);
    addSlider(layout2, "Gear 5 min", 0, 170, m_vehicule->gearMinSpeed[5], &m_vehicule->gearMinSpeed[5]);
    addSlider(layout2, "Gear 6 min", 0, 200, m_vehicule->gearMinSpeed[6], &m_vehicule->gearMinSpeed[6]);

    mainLayout->addWidget(group2);
}

void DevMenu::createBrakingGroup(QVBoxLayout* mainLayout)
{
    QGroupBox* group = new QGroupBox("Freinage");
    QVBoxLayout* layout = new QVBoxLayout(group);

    addSlider(layout, "Brake force", 0.001, 0.05, m_vehicule->brakeForce, &m_vehicule->brakeForce);
    addSlider(layout, "Brake decel", 0.1, 2.0, m_vehicule->brakeDecel, &m_vehicule->brakeDecel);
    addSlider(layout, "Drag on track", 0.95, 1.0, m_vehicule->dragOnTrack, &m_vehicule->dragOnTrack);
    addSlider(layout, "Drag on grass", 0.85, 0.99, m_vehicule->dragOnGrass, &m_vehicule->dragOnGrass);

    mainLayout->addWidget(group);
}
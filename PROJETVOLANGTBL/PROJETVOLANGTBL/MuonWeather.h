#pragma once
#ifndef MUONWEATHER_H
#define MUONWEATHER_H

#include <QObject>
#include "Vehicule.h"

class MuonWeather : public QObject
{
    Q_OBJECT

public:
    // rainyAt     : nb de muons (dans le cycle) pour RAINY  (défaut 3)
    // stormyAt    : nb de muons (dans le cycle) pour STORMY (défaut 7)
    // cycleLength : longueur totale du cycle avant retour à SUNNY (défaut 10)
    explicit MuonWeather(int rainyAt     = 3,
                         int stormyAt    = 7,
                         int cycleLength = 10,
                         QObject* parent = nullptr);

    // Appeler une fois par muon détecté
    void onMuon();

    // Remet le compteur à zéro
    void reset();

    int               getMuonCount()     const { return m_count; }
    Vehicule::Weather getCurrentWeather() const { return m_weather; }

signals:
    void weatherChanged(Vehicule::Weather newWeather);

private:
    int               m_count       = 0;
    int               m_rainyAt;
    int               m_stormyAt;
    int               m_cycleLength;
    Vehicule::Weather m_weather     = Vehicule::SUNNY;

    void applyIfChanged(Vehicule::Weather next);
};

#endif

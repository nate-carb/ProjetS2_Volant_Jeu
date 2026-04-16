#pragma once
#ifndef MUONWEATHER_H
#define MUONWEATHER_H

#include <QObject>
#include "Vehicule.h"

// ---------------------------------------------------------------------------
//  MuonWeather
//
//  Fait tourner la météo en boucle selon le nombre de muons détectés.
//
//  Cycle (basé sur cycleLength) :
//    pos  0 .. rainyAt-1  → SUNNY
//    pos  rainyAt .. stormyAt-1  → RAINY
//    pos  stormyAt .. cycleLength-1  → STORMY
//    pos  0 (wrap)  → SUNNY  → recommence
//
//  Exemple avec rainyAt=3, stormyAt=7, cycleLength=10 :
//    muon  3 → RAINY
//    muon  7 → STORMY
//    muon 10 → SUNNY   (retour)
//    muon 13 → RAINY   (2e tour)
//    ...
// ---------------------------------------------------------------------------
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

    // Remet le compteur à zéro (nouveau départ)
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

#endif // MUONWEATHER_H

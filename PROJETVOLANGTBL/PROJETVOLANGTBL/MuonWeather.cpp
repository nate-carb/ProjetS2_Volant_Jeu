#include "MuonWeather.h"
#include <QDebug>

MuonWeather::MuonWeather(int rainyAt, int stormyAt, int cycleLength, QObject* parent)
    : QObject(parent)
    , m_rainyAt(rainyAt)
    , m_stormyAt(stormyAt)
    , m_cycleLength(cycleLength)
{
}

void MuonWeather::onMuon()
{
    m_count++;

    // Position dans le cycle courant (0 .. cycleLength-1)
    int pos = m_count % m_cycleLength;

    Vehicule::Weather next;
    if      (pos < m_rainyAt)   next = Vehicule::SUNNY;   // début du cycle → beau temps
    else if (pos < m_stormyAt)  next = Vehicule::RAINY;
    else                        next = Vehicule::STORMY;

    qDebug() << "[MuonWeather] muon #" << m_count
             << " pos=" << pos << "/" << m_cycleLength;

    applyIfChanged(next);
}

void MuonWeather::reset()
{
    m_count   = 0;
    m_weather = Vehicule::SUNNY;
}

// ── Privé ────────────────────────────────────────────────────────────────────
void MuonWeather::applyIfChanged(Vehicule::Weather next)
{
    if (next == m_weather) return;
    m_weather = next;

    const char* name = (next == Vehicule::SUNNY)  ? "SUNNY"
                     : (next == Vehicule::RAINY)  ? "RAINY"
                     :                              "STORMY";
    qDebug() << "[MuonWeather] météo →" << name;

    emit weatherChanged(m_weather);
}

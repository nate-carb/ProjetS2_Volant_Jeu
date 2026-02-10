// Vehicule.cpp
#include "Vehicule.h"
#include <cmath>
#include <qDebug>
Vehicule::Vehicule()
{
    vitesse.x = 0.0f;
    vitesse.y = 0.0f;
    position.x = 0.0f;
    position.y = 0.0f;
    angle = 0.0f;  // Pointe vers la droite au départ

    accel = 0.0f;
    breaking = 0.0f;
    steering = 0.0f;
    carburant = 100.0f;
}

Vehicule::Vehicule(float x, float y)
{
    vitesse.x = 0.0f;
    vitesse.y = 0.0f;
    position.x = x;
    position.y = y;
    angle = 0.0f;

    accel = 0.0f;
    breaking = 0.0f;
    steering = 0.0f;
    carburant = 100.0f;
}

Vehicule::~Vehicule()
{
}

void Vehicule::update(float deltaTime)
{
    if (carburant <= 0) return;

    // Constantes physiques
    const float maxAcceleration = 20;  // m/s²
    const float turnRate = 2.0f;          // rad/s
    const float drag = 0.994f;             // Friction

    // 1. STEERING change l'angle de la voiture
    if (angle > 180.0f) { angle = 180.0f; }
    if (angle < -180.0f) { angle = -180.0f; }
    angle += steering * turnRate * deltaTime;
    // 2. ACCÉLÉRATION dans la direction de la voiture
    if (accel > 0) {
        float forceX = cos(angle) * maxAcceleration * accel;
        float forceY = sin(angle) * maxAcceleration * accel;

        vitesse.x += forceX * deltaTime;
        vitesse.y += forceY * deltaTime;
    }

    // 3. FREINAGE
    if (breaking > 0) {
        vitesse.x *= (1.0f - breaking * 0.05f);
        vitesse.y *= (1.0f - breaking * 0.05f);
    }

    // 4. FRICTION (ralentissement naturel)
    vitesse.x *= drag;
    vitesse.y *= drag;

    // 5. MISE À JOUR POSITION
    position.x += vitesse.x * deltaTime;
    position.y += vitesse.y * deltaTime;

    // 6. CONSOMMATION CARBURANT
    if (accel > 0) {
        carburant -= 0.01f * accel * deltaTime;
    }
	qDebug() << "Mon angle : " << angle << " et ma vitesse : " << getSpeed() << " m/s" << " et ma position : (" << position.x << ", " << position.y << ")";
}

float Vehicule::getSpeed() const
{
    return sqrt(vitesse.x * vitesse.x + vitesse.y * vitesse.y);
}
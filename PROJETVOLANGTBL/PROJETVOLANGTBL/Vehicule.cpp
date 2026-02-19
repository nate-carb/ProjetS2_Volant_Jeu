// Vehicule.cpp
#include "Vehicule.h"
#include <cmath>
#include <qDebug>
#include <algorithm>

using namespace std;
#define PI 3.14159265

Vehicule::Vehicule()
{
	vitesse.setX(0.0f);
    vitesse.setY(0.0f);
    position.setX(0.0f);
    position.setY(0.0f);
    angle = 0.0f;  // Pointe vers la droite au départ
	speed = 0.0f;
    accel = 0.0f;
    breaking = 0.0f;
    steering = 0.0f;
    carburant = 100.0f;
    angularVelocity = 0.0f;
}

Vehicule::Vehicule(float x, float y)
{
    vitesse.setX(0.0f);
    vitesse.setY(0.0f);
	speed = 0.0f;
    position.setX(x);
    position.setY(y);
    angle = 0.0f;

    accel = 0.0f;
    breaking = 0.0f;
    steering = 0.0f;
    carburant = 100.0f;
    angularVelocity = 0.0f;
}

Vehicule::~Vehicule()

{
}

//void Vehicule::update(float deltaTime)
//{
//    if (carburant <= 0) return;
//
//    // Constantes physiques
//    const float maxAcceleration = 25;  // m/s²
//    const float turnRate = 2.0f;          // rad/s
//    const float drag = 0.9978f;             // Friction
//    // 1. STEERING change l'angle de la voiture
//    //if (angle > PI) { angle = 0; }
//    //if (angle < -PI) { angle = 0; }
//    angle += steering * turnRate * deltaTime;
//    // ACCÉLÉRATION dans la direction de la voiture
//    if (accel >= 0) {
//        float forceX = cos(angle) * maxAcceleration * accel;
//        float forceY = sin(angle) * maxAcceleration * accel;
//
//        vitesse.setX(vitesse.x()+ forceX * deltaTime);
//        vitesse.setY(vitesse.y()+ forceY * deltaTime);
//    }
//
//    // FREINAGE
//    if (breaking > 0) {
//        vitesse.setX(vitesse.x()*(1.0f - breaking * 0.035f));
//        vitesse.setY(vitesse.y()*(1.0f - breaking * 0.035f));
//    }
//
//    // 4. FRICTION (ralentissement naturel)
//    vitesse.setX(vitesse.x()*drag);
//    vitesse.setY(vitesse.y()*drag);
//
//    // 5. MISE À JOUR POSITION
//    position.setX(position.x()+vitesse.x() * deltaTime);
//    position.setY(position.y()+vitesse.y() * deltaTime);
//
//    // 6. CONSOMMATION CARBURANT
//    if (accel > 0) {
//        carburant -= 0.01f * accel * deltaTime;
//    }
//	qDebug() << "Mon angle : " << angle << " et ma vitesse : " << getSpeed() << " m/s" << " et ma position : (" << position.x() << ", " << position.y() << ")";
//}

void Vehicule::update(float deltaTime)
{
    if (carburant <= 0) return;

    const float maxAcceleration = 35.0f;
    const float drag_on_track = 0.999f;
    const float drag_on_grass = 0.96;
    // ===== PARAMÈTRES DE CONDUITE =====
    const float maxTurnSpeed = 4.0f;      // rad/s
    const float turnResponsiveness = 8.0f;

    // 1) ACCÉLÉRATION agit sur la vitesse SCALAIRE

    if (accel > 0) {
        speed += maxAcceleration * accel * deltaTime;
    }

    // 2) FREINAGE agit sur speed
    if (breaking > 0) {
        speed *= (1.0f - breaking * 0.01f);
        speed = speed - 0.3f;
		speed = std::max(speed, 0.0f);
    }
        
    // 3) FRICTION naturelle
    speed *= drag;

    // 4) STEERING -> angular velocity (dépend de la vitesse)
    float speedValue = vitesse.length();

    if (speedValue > 0.1f)
    {
        float speedNorm = std::clamp(speedValue / 80.0f, 0.0f, 1.0f);

        // courbe en cloche (0 ? 1 ? 0)
        float bell = 4.0f * speedNorm * (1.0f - speedNorm);

        const float minTurnFactor = 0.20f;  // tourne encore à haute vitesse
        float turnFactor = minTurnFactor + bell * (1.0f - minTurnFactor);

        float rotationAmount = steering * turnFactor * 2.5f * deltaTime;

        float cs = std::cos(rotationAmount);
        float sn = std::sin(rotationAmount);

        float vx = vitesse.x();
        float vy = vitesse.y();

        // rotation du vecteur vitesse
        vitesse.setX(vx * cs - vy * sn);
        vitesse.setY(vx * sn + vy * cs);
        angle = std::atan2(vitesse.y(), vitesse.x());
    }

    // 5) Reconstruit la vitesse à partir de l’angle
    vitesse.setX(std::cos(angle) * speed);
    vitesse.setY(std::sin(angle) * speed);

    // 6) Position

    position.setX(position.x() + vitesse.x() * deltaTime);
    position.setY(position.y() + vitesse.y() * deltaTime);

    // 7) Carburant
    if (accel > 0) {
        carburant -= 0.01f * accel * deltaTime;
    }

    qDebug() << "Mon angle : " << angle << " et ma vitesse : " << getSpeed() << " m/s" << " et ma position : (" << position.x() << ", " << position.y() << ")";
}


float Vehicule::getSpeed() const
{
    return vitesse.length();
}
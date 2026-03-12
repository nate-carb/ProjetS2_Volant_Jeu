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
	is_on_grass = false;
	is_on_track = true;
    nos = 100.0f;
    boosting = false;
    tireWear = 100.0f;
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
    is_on_grass = false;
    is_on_track = true;
    nos = 100.0f;
    boosting = false;
    tireWear = 100.0f;
}

Vehicule::~Vehicule()

{
}

void Vehicule::update(float deltaTime)
{
    if (carburant <= 0) return;

    const float maxAcceleration = 45.0f; // ajuster vitesse max
    const float drag_on_track = 0.999f;
    const float drag_on_grass = 0.96f;
    // ===== PARAMÈTRES DE CONDUITE =====
    const float maxTurnSpeed = 4.0f;      // rad/s
    const float turnResponsiveness = 8.0f;

    // 1) ACCÉLÉRATION agit sur la vitesse SCALAIRE

    if (accel > 0) {
        speed += maxAcceleration * accel * deltaTime;
    }

    // 2) NOS boost la vitesse du véhicule
    const float nosForce = 50.0f;       // force du boost (on peut ajuster en consequece apres tests)
    const float nosDrain = 15.0f;       //consommation de nos/seconde
    if (boosting && nos > 0.0f) {
        speed += nosForce * deltaTime;
        nos -= nosDrain * deltaTime;
        nos = std::max(nos, 0.0f);      //empeche que le nos aille sous 0
    }

    const float nosRegen = 2.0f;        // % par seconde (on peut ajuster en consequece apres tests)

    if (!boosting && nos < 100.0f) {
        nos += nosRegen * deltaTime;
        nos = std::min(nos, 100.0f);        // empêche que le nos dépasse 100
    }

    // Réduit la vitesse max selon l'usure
    float maxSpeed = 200.0f + (tireWear / 100.0f) * 100.0f;
    if (boosting && nos > 0.0f) {
        maxSpeed *= 1.25f;  // 125% de la vitesse max quand NOS actif
    }
    if (speed > maxSpeed) {
        speed = maxSpeed;
    }

    // 3) FREINAGE agit sur speed
    if (breaking > 0) {
        speed *= (1.0f - breaking * 0.01f);
        speed = speed - 0.3f;
		speed = std::max(speed, 0.0f);
    }
        
    // 3) FRICTION naturelle
    if (is_on_grass) {
        speed *= drag_on_grass;
    }
    else if (is_on_track) {
        speed *= drag_on_track;
    }

    // 5) STEERING -> angular velocity (dépend de la vitesse)
    float speedValue = vitesse.length();

    if (speedValue > 0.1f)
    {
        float speedNorm = std::clamp(speedValue / 100.0f, 0.0f, 1.0f); // ajuster turning rate a 100

        // courbe en cloche (0 ? 1 ? 0)
        float bell = 4.0f * speedNorm * (1.0f - speedNorm);

        const float minTurnFactor = 0.35f;  // tourne encore à haute vitesse
        float turnFactor = minTurnFactor + bell * (1.0f - minTurnFactor);

        float wetFactor = 1.0f;
        if (weather == RAINY)  wetFactor = 0.9f;   // tourne moins bien
        if (weather == STORMY) wetFactor = 0.8f;  // glisse beaucoup

        float rotationAmount = steering * turnFactor * 2.5f * wetFactor * deltaTime;

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

    // 7) Position

    position.setX(position.x() + vitesse.x() * deltaTime);
    position.setY(position.y() + vitesse.y() * deltaTime);

    // 8) Carburant
    if (speed > 0.1f) {
        carburant -= 0.5f * deltaTime;         // consommation de base en mouvement
    }
    if (accel > 0 || boosting) {
        carburant -= 0.01f * deltaTime;        // consommation en accel et boost
    }

    carburant = std::max(carburant, 0.0f);  // empêche d'aller sous 0

    qDebug() << "Mon angle : " << angle << " et ma vitesse : " << getSpeed() << " m/s" << " et ma position : (" << position.x() << ", " << position.y() << ")";
	qDebug() << "Surface: " << (is_on_grass ? "Herbe" : (is_on_track ? "Piste" : "Autre"));
    qDebug() << "| Carburant:" << (int)carburant << "%"
        << "| NOS:" << (int)nos << "%";

    // USURE DES PNEUS
    const float wearRate = 0.5f;  // usure par seconde, ajuste au goût
    if (speed > 0.1f) {
        tireWear -= wearRate * deltaTime;
        tireWear = std::max(tireWear, 0.0f);
    }

    
}


float Vehicule::getSpeed() const
{
    return vitesse.length();
}
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

    // 1) ACCÉLÉRATION avec gear
    if (accel > 0) {
        if (speed < gearMinSpeed[gear]) {
            speed -= 5.0f * deltaTime;
            speed = std::max(speed, 0.0f);
        }
        else {
            speed += maxAcceleration * accel * deltaTime;
        }
    }

    // 2) NOS
    if (boosting && nos > 0.0f) {
        speed += nosForce * deltaTime;
        nos -= nosDrain * deltaTime;
        nos = std::max(nos, 0.0f);
    }
    if (!boosting && nos < 100.0f) {
        nos += nosRegen * deltaTime;
        nos = std::min(nos, 100.0f);
    }

    // 3) Brider à la vitesse max du gear
    float maxSpeed = gearMaxSpeed[gear];
    if (boosting && nos > 0.0f) maxSpeed *= 1.25f;
    speed = std::min(speed, maxSpeed);

    // 4) FREINAGE
    if (breaking > 0) {
        speed *= (1.0f - breaking * brakeForce);
        speed -= brakeDecel * breaking * deltaTime;
        speed = std::max(speed, 0.0f);
    }

    // 5) FRICTION
    speed *= is_on_grass ? dragOnGrass : dragOnTrack;

    // 6) STEERING
    float speedValue = vitesse.length();
    if (speedValue > 0.1f) {
        float speedNorm = std::clamp(speedValue / 100.0f, 0.0f, 1.0f);
        float bell = 4.0f * speedNorm * (1.0f - speedNorm);
        float turnFactor = minTurnFactor + bell * (maxTurnFactor - minTurnFactor);

        float wetFactor = 1.0f;
        if (weather == RAINY)  wetFactor = 0.9f;
        if (weather == STORMY) wetFactor = 0.8f;

        float rotationAmount = steering * turnFactor * wetFactor * deltaTime;

        float cs = std::cos(rotationAmount);
        float sn = std::sin(rotationAmount);
        float vx = vitesse.x();
        float vy = vitesse.y();

        vitesse.setX(vx * cs - vy * sn);
        vitesse.setY(vx * sn + vy * cs);
        angle = std::atan2(vitesse.y(), vitesse.x());
    }

    // 7) Reconstruit vitesse depuis angle
    vitesse.setX(std::cos(angle) * speed);
    vitesse.setY(std::sin(angle) * speed);

    // 8) Position
    position.setX(position.x() + vitesse.x() * deltaTime);
    position.setY(position.y() + vitesse.y() * deltaTime);

    // 9) Carburant
    if (speed > 0.1f)        carburant -= 0.5f * deltaTime;
    if (accel > 0 || boosting) carburant -= 0.01f * deltaTime;
    carburant = std::max(carburant, 0.0f);

    // 10) RPM
    float targetRpm = minRpm + (speed / gearMaxSpeed[gear]) * maxRpm;
    rpm += (targetRpm - rpm) * 0.1f;
    rpm = std::clamp(rpm, minRpm, maxRpm);

    // 11) Usure pneus
    if (speed > 0.1f) {
        tireWear -= 0.5f * deltaTime;
        tireWear = std::max(tireWear, 0.0f);
    }
}

float Vehicule::getSpeed() const {
    return vitesse.length();
}

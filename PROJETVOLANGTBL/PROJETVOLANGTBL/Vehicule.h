#include <QVector2D>
#pragma once

struct Vector2 {
    float x;
    float y;
};

class Vehicule {
public:
    Vehicule();
    Vehicule(float x, float y);
    ~Vehicule();

    void update(float deltaTime);

    // Getters/Setters existants
    QVector2D getVitesse() const { return vitesse; }
    void setVitesse(float vx, float vy) { vitesse.setX(vx); vitesse.setY(vy); }
    float getAccel() const { return accel; }
    void setAccel(float a) { accel = a; }
    float getCarburant() const { return carburant; }
    void setCarburant(float c) { carburant = c; }
    QVector2D getPosition() const { return position; }
    void setPosition(float x, float y) { position.setX(x); position.setY(y); }
    float getSteering() const { return steering; }
    void setSteering(float s) { steering = s; }
    float getAngle() const { return angle; }
    float getSpeed() const;
    float getBreaking() const { return breaking; }
    void setBreaking(float b) { breaking = b; }
    float getNos() const { return nos; }
    void setNos(float n) { nos = n; }
    void setBoosting(bool b) { boosting = b; }
    bool isBoosting() const { return boosting; }
    float getTireWear() const { return tireWear; }
    void setTireWear(float t) { tireWear = t; }
    float getRpm() const { return rpm; }
	float getMinRpm() const { return minRpm; }
	float getMaxRpm() const { return maxRpm; }

    // Getters/Setters DevMenu
    float getBrakeForce() const { return brakeForce; }
    void setBrakeForce(float f) { brakeForce = f; }
    float getBrakeDecel() const { return brakeDecel; }
    void setBrakeDecel(float d) { brakeDecel = d; }
    float getDragOnTrack() const { return dragOnTrack; }
    void setDragOnTrack(float d) { dragOnTrack = d; }
    float getDragOnGrass() const { return dragOnGrass; }
    void setDragOnGrass(float d) { dragOnGrass = d; }

    // Gear
    void shiftUp() { if (gear < maxGear) gear++; }
    void shiftDown() {
        if (gear > 1 && speed <= gearMaxSpeed[gear - 1])
            gear--;
    }
    int getGear() { return gear; }

    // Weather
    enum Weather { SUNNY, RAINY, STORMY };
    void setWeather(Weather w) { weather = w; }
    Weather getWeather() const { return weather; }

    bool is_on_track = true;
    bool is_on_grass = false;

    // ?? Paramètres tunable (publics pour DevMenu) ????????????
    float maxAcceleration = 45.0f;
    float turnResponsiveness = 8.0f;
    float minTurnFactor = 0.35f;
    float maxTurnFactor = 2.5f;
    float nosForce = 50.0f;
    float nosDrain = 15.0f;
    float nosRegen = 2.0f;
    float brakeForce = 0.01f;
    float brakeDecel = 0.3f;
    float dragOnTrack = 0.999f;
    float dragOnGrass = 0.96f;
    float gearMinSpeed[7] = { 0,   0,  30,  60, 100, 140, 180 };
    float gearMaxSpeed[7] = { 0,  40,  80, 120, 160, 200, 240 };

private:
    QVector2D vitesse;
    QVector2D position;
    float angle = 0.0f;
    float speed = 0.0f;
    float accel = 0.0f;
    float steering = 0.0f;
    float breaking = 0.0f;
    float carburant = 100.0f;
    float angularVelocity = 0.0f;
    float nos = 100.0f;
    bool  boosting = false;
    float tireWear = 100.0f;
    float rpm = 800.0f;
    float minRpm = 800.0f;
    float maxRpm = 8000.0f;
    int   gear = 1;
    int   maxGear = 6;
    Weather weather = SUNNY;

    friend class DevMenu;
};
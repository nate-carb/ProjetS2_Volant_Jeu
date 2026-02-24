// Vehicule.h
#include <QVector2D>
#pragma once
struct Vector2
{
    float x;
    float y;
};

class Vehicule
{
public:
    Vehicule();
    Vehicule(float x, float y);
    ~Vehicule();

    void update(float deltaTime);  // Nouvelle fonction principale

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

    float getAngle() const { return angle; }  // NOUVEAU
    float getSpeed() const;  // NOUVEAU - vitesse en m/s

	float getBreaking() const { return breaking; }
	void setBreaking(float b) { breaking = b; }

    float getNos() const { return nos; }
    void setNos(float n) { nos = n; }

    void setBoosting(bool b) { boosting = b; }
    bool isBoosting() const { return boosting; }

private:
    QVector2D vitesse;
    QVector2D position;
    float angle;  // direction de la voiture en radians
	float speed;  // vitesse scalaire en m/s
    float accel;     // 0 à 1	
    float steering;  // -1 à 1
    float breaking;  // 0 à 1 
    float carburant; // en pourcentage
    float angularVelocity;    // vitesse de rotation
    float nos;          // réservoir de NOS, 0 à 100%
    bool boosting;      // est-ce-qu'on boost live ou non
};
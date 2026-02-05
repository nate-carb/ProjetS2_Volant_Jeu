// Vehicule.h
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

    Vector2 getVitesse() const { return vitesse; }
    void setVitesse(float vx, float vy) { vitesse.x = vx; vitesse.y = vy; }

    float getAccel() const { return accel; }
    void setAccel(float a) { accel = a; }

    float getCarburant() const { return carburant; }
    void setCarburant(float c) { carburant = c; }

    Vector2 getPosition() const { return position; }
    void setPosition(float x, float y) { position.x = x; position.y = y; }

    float getSteering() const { return steering; }
    void setSteering(float s) { steering = s; }

    float getAngle() const { return angle; }  // NOUVEAU
    float getSpeed() const;  // NOUVEAU - vitesse en m/s

	float getBreaking() const { return breaking; }
	void setBreaking(float b) { breaking = b; }

private:
    Vector2 vitesse;
    Vector2 position;
    float angle;  // NOUVEAU - direction de la voiture en radians

    float accel;     // 0 à 1	
    float steering;  // -1 à 1
    float breaking;  // 0 à 1 
    float carburant; // en pourcentage
};
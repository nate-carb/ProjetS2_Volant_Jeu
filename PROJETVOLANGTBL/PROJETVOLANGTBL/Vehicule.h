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

	bool deplacement();

	Vector2 getVitesse() const { return vitesse; }
	void setVitesse(float vx, float vy) { vitesse.x = vx; vitesse.y = vy; }

	float getAccel() const { return accel; }
	void setAccel(float a) { accel = a; }

	float getCarburant() const { return carburant; }
	void setCarburant(float c) { carburant = c; }

	Vector2 getPosition() const { return position; }
	void setPosition(float x, float y) { position.x = x; position.y = y; }

	// Add getter and setter for steering
	float getSteering() const { return steering; }
	void setSteering(float s) { steering = s; }



private:
	Vector2 vitesse; 
	Vector2 position;
	float accel; //0 a 1	
	float steering; //-1 a 1
	float breaking; //0 a 1 
	float carburant; // en pourcentage
	void calculervitesse();
	/*int gear*/



};


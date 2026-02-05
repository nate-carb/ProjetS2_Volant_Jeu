#include "Vehicule.h"

Vehicule::Vehicule() 
{
	vitesse.x = 0.0f;
	vitesse.y = 0.0f;
	accel = 0.0f; //0 a 1
	breaking = 0.0f; //0 a 1
	steering = 0.0f; //-1 a 1
	carburant = 100.0f;
	position.x = 0.0f;
	position.y = 0.0f;

}

Vehicule::Vehicule(float x, float y) 
{
	vitesse.x = 0.0f;
	vitesse.y = 0.0f;
	accel = 0.0f;
	carburant = 100.0f;
	position.x = x;
	position.y = y;
}
Vehicule::~Vehicule() 
{
}
bool Vehicule::deplacement() 
{
	if (carburant <= 0) return false;
	calculervitesse();
	position.x = position.x + vitesse.x ;
	position.y = position.y + vitesse.y;
	return true;
}
void Vehicule::calculervitesse() 
{
	vitesse.x = vitesse.x + accel;
	vitesse.y = vitesse.y + accel;

}

#pragma once
class GameEngine
{
	public:
	GameEngine();
	~GameEngine();
	void start();
	void update();
	void stop();
	
private:
	float deltaTime = 1 / fps;
	float currentTime;
	float fps;

};


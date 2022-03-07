#pragma once
#include "SDL.h"
#include "SDL_ttf.h"
#include "Utils.h"
#include <iostream>
#include <fstream>

class Game
{
public:
	/*
	* Game constructor
	*/
	Game();

	/*
	* Init the game
	*/
	bool Initialize();

	/*
	* Run the game loop until the game is over
	*/
	void RunLoop();

	/*
	* Shutdown the game
	*/
	void Shutdown();

private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();
	void UpdateBallVelocity();
	void SaveHighScore();
	void LoadHighScore();

	// Window created by SDL
	SDL_Window* Window;
	// Renderer for 2D drawing
	SDL_Renderer* Renderer;
	// Number of ticks since start of game
	Uint32 TicksCount;
	// Game should continue to run
	bool IsRunning;

	// Game objs
	int PaddleDir; // Direction of paddle
	Vector2_t PaddlePos; // Position of paddle
	Vector2_t BallPos; // Position of ball
	Vector2_t BallVel; // Velocity of ball
};



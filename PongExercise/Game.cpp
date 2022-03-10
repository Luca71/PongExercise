#include "Game.h"
#include <string>
using namespace std;

#define WIN_BACKGROUND_COLOR 0, 46, 120, 255
#define WALL_COLOR 0, 11, 28, 250
#define PADDLE_COLOR 250, 250, 250, 255
#define FONT_COLOR 250, 250, 250, 255
#define FONT_SIZE 100

// Window size
const int WinWidth = 1024;
const int WinHeight = 768;

// Wall size
const int WallThikness = 15;

// Paddle size and speed
const int PaddleWidth = 15;
const float PaddleHeight = 100.0f;
const float PaddleSpeed = 300.0f;
const float PaddleStartPosX = 40;

// Ball size and speed
const int BallWidth = 15;
const int BallHeight = 15;
float BallSpeed = 200.0f;
int LoopCount = 0;

// Mouse pos
int MousePosY;

// Font Score
int Score = 0;
TTF_Font* Orbitron;
SDL_Color FontColor = { FONT_COLOR };
SDL_Surface* ScoreSurface;
SDL_Texture* ScoreTexture;
SDL_Rect ScoreRect = { (WinWidth - 290), 20, 250, 50 };
SDL_Rect HighScoreRect = {(WinWidth - 290), 70, 150, 20 };

// Save Score
string ScoreCompositText;
string HighScore;


Game::Game() :	Window(nullptr), Renderer(nullptr),
				TicksCount(0), IsRunning(true), PaddleDir(0), PaddlePos(Vector2_t{ PaddleStartPosX , 0}), 
				BallPos(Vector2_t{ WinWidth * 0.5f , WinHeight * 0.5f }), BallVel(Vector2_t{1 , 1})
{
}

bool Game::Initialize()
{
	// Init SDL
	if (TTF_Init() == -1) {
		SDL_Log("TTF_Init: %s\n", TTF_GetError());
		return false;
	}

	int SdlInit = SDL_Init(SDL_INIT_VIDEO);
	if (SdlInit != 0)
	{
		SDL_Log("Unable to init SDL: %s", SDL_GetError());
		return false;
	}

	// Create game window
	Window = SDL_CreateWindow("One Player Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WinWidth, WinHeight, 0);
	if (!Window)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}

	// Create SDL renderer
	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!Renderer)
	{
		SDL_Log("Failed to create renderer: %s", SDL_GetError());
		return false;
	}

	LoadHighScore();

	return true;
}

void Game::RunLoop()
{
	while (IsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::ProcessInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				IsRunning = false;
				break;
			
			case SDL_MOUSEMOTION:
				//MousePos.x = event.motion.x;
				MousePosY = event.motion.y;
				break;
		}
	}

	// Hide mouse cursor
	SDL_ShowCursor(SDL_DISABLE);

	// Get state of keyboard
	const Uint8* state = SDL_GetKeyboardState(NULL);

	// If escape is pressed, also end loop
	if (state[SDL_SCANCODE_ESCAPE])
	{
		IsRunning = false;
	}

	// Update paddle direction based on W/S keys
	PaddleDir = 0;
	if (state[SDL_SCANCODE_W])
	{
		PaddleDir -= 1;
	}
	if (state[SDL_SCANCODE_S])
	{
		PaddleDir += 1;
	}
}

void Game::UpdateBallVelocity()
{
	if (LoopCount > 60)
	{
		BallSpeed += 10.0f;
		LoopCount = 1;

		Score += 10;

		return;
	}
	LoopCount++;
}

void Game::UpdateGame()
{
	// Wait until 16ms has elapsed since last frame (60 fps)
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), TicksCount + 16));

	float deltaTime = (SDL_GetTicks() - TicksCount) / 1000.0f;

	// Clamp maximum delta time value
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	// Update tick counts (for next frame)
	TicksCount = SDL_GetTicks();

	// Update paddle position based on direction
	/*
	if (PaddleDir != 0)
	{
		PaddlePos.y += PaddleDir * PaddleSpeed * deltaTime;
		// Make sure paddle doesn't move off screen!
		if (PaddlePos.y < (PaddleHeight * 0.5f + PaddleWidth))
		{
			PaddlePos.y = PaddleHeight * 0.5f + PaddleWidth;
		}
		else if (PaddlePos.y > (WinHeight - PaddleHeight * 0.5f - PaddleWidth))
		{
			PaddlePos.y = WinHeight - PaddleHeight * 0.5f - PaddleWidth;
		}
	}
	*/

	// Paddle position controlled by mouse
	PaddlePos.y = MousePosY;
	if (PaddlePos.y < (PaddleHeight * 0.5f + PaddleWidth))
	{
		PaddlePos.y = PaddleHeight * 0.5f + PaddleWidth;
	}
	else if (PaddlePos.y > (WinHeight - PaddleHeight * 0.5f - PaddleWidth))
	{
		PaddlePos.y = WinHeight - PaddleHeight * 0.5f - PaddleWidth;
	}

	// Update ball position based on ball velocity
	BallPos.x += BallVel.x * BallSpeed * deltaTime;
	BallPos.y += BallVel.y * BallSpeed * deltaTime;

	// Paddle collision
	float diff = PaddlePos.y - BallPos.y;

	// Take absolute value of difference
	diff = (diff > 0.0f) ? diff : -diff;
	if (
		// Our y-difference is small enough
		diff <= PaddleHeight * 0.5f &&
		// We are in the correct x-position
		BallPos.x <= (PaddlePos.x + PaddleWidth) && BallPos.x >= 20.0f &&
		// The ball is moving to the left
		BallVel.x < 0.0f)
	{
		BallVel.x *= -1.0f;
	}

	// Did the ball go off the screen? (if so, end game)
	// Wall collisions
	else if (BallPos.x <= 0.0f)
	{
		int HScore = stoi(HighScore);
		if (Score > HScore)
		{
			SaveHighScore();
		}
		IsRunning = false;
	}
	// Did the ball collide with the right wall?
	else if (BallPos.x >= (WinWidth - WallThikness) && BallVel.x > 0.0f)
	{
		BallVel.x *= -1.0f;
	}
	// Did the ball collide with the top wall?
	if (BallPos.y <= WallThikness && BallVel.y < 0.0f)
	{
		BallVel.y *= -1;
	}
	// Did the ball collide with the bottom wall?
	else if (BallPos.y >= (WinHeight - WallThikness) && BallVel.y > 0.0f)
	{
		BallVel.y *= -1;
	}

	UpdateBallVelocity();
}

void Game::GenerateOutput()
{
	// Set window background color
	SDL_SetRenderDrawColor(Renderer, WIN_BACKGROUND_COLOR);

	// Clear back buffer
	SDL_RenderClear(Renderer);

	/* Draw walls */
	SDL_SetRenderDrawColor(Renderer, WALL_COLOR);
	// Draw top wall
	SDL_Rect wall{0, 0, WinWidth, WallThikness};
	SDL_RenderFillRect(Renderer, &wall);
	// Draw bottom wall
	wall.y = WinHeight - WallThikness;
	SDL_RenderFillRect(Renderer, &wall);
	// Draw right wall
	wall.x = WinWidth - WallThikness;
	wall.y = 0;
	wall.w = WallThikness;
	wall.h = WinWidth;
	SDL_RenderFillRect(Renderer, &wall);

	// Draw paddle
	SDL_SetRenderDrawColor(Renderer, PADDLE_COLOR);
	SDL_Rect paddle{
		static_cast<int>(PaddlePos.x),
		static_cast<int>(PaddlePos.y - PaddleHeight * 0.5f),
		PaddleWidth,
		static_cast<int>(PaddleHeight)
	};
	SDL_RenderFillRect(Renderer, &paddle);

	// Draw ball
	SDL_Rect ball{
		static_cast<int>(BallPos.x - BallWidth * 0.5f),
		static_cast<int>(BallPos.y - BallHeight * 0.5f),
		BallWidth,
		BallHeight
	};
	SDL_RenderFillRect(Renderer, &ball);

	// Draw Score
	Orbitron = TTF_OpenFont("Assets\\Orbitron-Regular.ttf", FONT_SIZE);
	string ScoreFixText = "SCORE: ";
	string ScoreValue = to_string(Score);
	ScoreCompositText = ScoreFixText + ScoreValue;
	const char* str = ScoreCompositText.c_str();
	ScoreSurface = TTF_RenderText_Solid(Orbitron, str, FontColor);
	if(ScoreSurface == NULL)
	{
		SDL_Log("Failed to create surface: %s", SDL_GetError());
	}
	ScoreTexture = SDL_CreateTextureFromSurface(Renderer, ScoreSurface);
	if (ScoreTexture == NULL)
	{
		SDL_Log("Failed to create texture: %s", SDL_GetError());
	}
	SDL_RenderCopy(Renderer, ScoreTexture, NULL, &ScoreRect);

	// Draw High Score
	string Prefix = "HIGHSCORE: ";
	string CompositeHighScore = Prefix + HighScore;
	str = CompositeHighScore.c_str();
	ScoreSurface = TTF_RenderText_Solid(Orbitron, str, FontColor);
	if (ScoreSurface == NULL)
	{
		SDL_Log("Failed to create surface: %s", SDL_GetError());
	}
	ScoreTexture = SDL_CreateTextureFromSurface(Renderer, ScoreSurface);
	if (ScoreTexture == NULL)
	{
		SDL_Log("Failed to create texture: %s", SDL_GetError());
	}
	SDL_RenderCopy(Renderer, ScoreTexture, NULL, &HighScoreRect);

	SDL_DestroyTexture(ScoreTexture);
	SDL_FreeSurface(ScoreSurface);

	// Swap front buffer and back buffer
	SDL_RenderPresent(Renderer);
}

void Game::SaveHighScore()
{
	ofstream ScoreToSave("Assets/Savefile.txt");
	ScoreToSave << to_string(Score);
	ScoreToSave.close();
}

void Game::LoadHighScore()
{
	ifstream ScoreToLoad("Assets/Savefile.txt");
	getline(ScoreToLoad, HighScore);
	ScoreToLoad.close();
}

void Game::Shutdown()
{
	SDL_DestroyRenderer(Renderer);
	SDL_DestroyWindow(Window);
	SDL_Quit();
}

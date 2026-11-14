#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cenui.h>
#include <vector>

/*enum class GameStatus {
	Menu,
	Playing,
	GameOver
};
*/

struct GameState; // Forward Declaration for GameStatus

struct GameStatus {
	void (*keyPressed)(GameState& game, const sf::Event::KeyPressed* event) = nullptr;
	void (*mouseButtonPressed)(GameState& game, const sf::Event::MouseButtonPressed* event) = nullptr;
	void (*draw)(const GameState& game, sf::RenderWindow& window) = nullptr;
	void (*tick)(GameState& game, sf::RenderWindow& window) = nullptr;
	void (*enter)(const GameState& game) = nullptr;
	void (*exit)(const GameState& game) = nullptr;
};

struct Bird {
	sf::Vector2f pos; // 0 will be treated as the middle of the screen to make it easier to handle screen resizing;
	float yVel = 0; // we don't have to recalc the middle on screen resize. As a result conversions need to happen
			// between SFML positive-is-down space and "normal" euclidean plane space.
};

struct Pipe {
	float xPos = 0;
	float gapHeight = 200.0f;
	float gapPos = 0;
	bool passed = false;
};

struct GameState {
	GameState() = default;
	GameStatus status;

	sf::RenderWindow* window = nullptr;
	cenui::FrameManager ui;

	unsigned int score = 0;
	bool alive = false;
	unsigned int health = 3;
	
	Bird bird;
	std::vector<Pipe> pipes;
	
	sf::Clock clock;
	sf::Time lastPipeSpawn;
	sf::Time lastTick;
	sf::Time lastHitTime;
	float scrollDistance = 0.0f; // how far the world has moved, used for the background
	
	int bgTheme = 0; // which background theme is picked in the menu, doesnt get reset
	int pipeSkin = 0; // same thing for the 
	int birdSkin = 0;
	
	// Sounds and Music
	sf::SoundBuffer flapBuffer;
	sf::Sound flapSound{flapBuffer};
	sf::Music menuMusic;
	sf::Music gameOverMusic;
	sf::SoundBuffer hitBuffer;
	sf::Sound hitSound{hitBuffer};
	sf::SoundBuffer coinBuffer;
	sf::Sound coinSound{coinBuffer};
};

void setGameStatus(GameState& game, GameStatus status);
void resetGame(GameState& game);
void playGame(GameState& game);

#endif

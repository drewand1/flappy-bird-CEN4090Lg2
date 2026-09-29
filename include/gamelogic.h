#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
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
	GameStatus status;
	Bird bird;
	std::vector<Pipe> pipes;
	unsigned int score = 0;
	sf::Clock clock;
	sf::Time lastPipeSpawn;
	sf::Time lastTick;
	bool alive = true;
	unsigned int health = 3;
	sf::Time lastHitTime;
	sf::RenderWindow* window = nullptr;
};

void runTickLogic(GameState& game, sf::Window& window);
void resetGame(GameState& game);
void playGame(GameState& game);

#endif

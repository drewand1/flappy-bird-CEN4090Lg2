#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <SFML/Window.hpp>
#include <vector>

struct Bird {
	sf::Vector2f pos; // 0 will be treated as the middle of the screen to make it easier to handle screen resizing;
	float yVel = 0; // we don't have to recalc the middle on screen resize. As a result conversions need to happen
			// between SFML positive-is-down space and "normal" euclidean plane space.
};

struct Pipe {
	float xPos = 0;
	float gapHeight = 10.0f;
	float gapPos = 0;
};

struct GameState {
	Bird bird;
	std::vector<Pipe> pipes;
	unsigned int score = 0;
	sf::Clock clock;
	sf::Time lastPipeSpawn;
	sf::Time lastTick;
	bool alive = true;
};

void runTickLogic(GameState& game, sf::Window& window);

#endif
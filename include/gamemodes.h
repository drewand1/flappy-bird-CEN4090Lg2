#ifndef GAMEMODES_H
#define GAMEMODES_H

#include <SFML/Graphics.hpp>
#include <string>
#include "gamelogic.h"

inline constexpr int GAME_MODE_COUNT = 4;

enum GameModeIndex {
	MODE_CLASSIC = 0,
	MODE_FLAPPY_CRUSH = 1,
	MODE_DARTS = 2,
	MODE_FLAP_HUNT = 3
};

extern const std::string gameModeNames[GAME_MODE_COUNT];

// Stub handlers for Flappy Crush and Darts
void keyPressedStubMode(GameState& game, const sf::Event::KeyPressed* event);
void drawStubMode(const GameState& game, sf::RenderWindow& window);

// Flap Hunt functions
void initFlapHunt(GameState& game);
void runFlapHuntTick(GameState& game, sf::Window& window);
void keyPressedFlapHunt(GameState& game, const sf::Event::KeyPressed* event);
void mousePressedFlapHunt(GameState& game, const sf::Event::MouseButtonPressed* event);
void drawFlapHunt(const GameState& game, sf::RenderWindow& window);

namespace GameStatuses {
	extern GameStatus flappyCrush;
	extern GameStatus darts;
	extern GameStatus flapHunt;
}

#endif
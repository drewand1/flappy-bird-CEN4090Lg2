#ifndef BIRDSELECT_H
#define BIRDSELECT_H

#include <SFML/Graphics.hpp>
#include "gamelogic.h"

constexpr int BIRD_SKIN_COUNT = 8;

// Texture for the picked bird. Loads all of them the first time it's called.
const sf::Texture& birdSkinTexture(int skin);

// The little button in the corner of the main menu. drawBirdButton shows it
// and mouseBirdButton opens the picker when it gets clicked.
void drawBirdButton(const GameState& game, sf::RenderWindow& window);
void mouseBirdButton(GameState& game, const sf::Event::MouseButtonPressed* event);

// The picker screen itself
void keyPressedBirdSelect(GameState& game, const sf::Event::KeyPressed* event);
void mouseBirdSelect(GameState& game, const sf::Event::MouseButtonPressed* event);
void drawBirdSelect(const GameState& game, sf::RenderWindow& window);

namespace GameStatuses {
	extern GameStatus birdSelect;
}

#endif
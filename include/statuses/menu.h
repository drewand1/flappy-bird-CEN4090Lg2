#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
#include "gamelogic.h"

void initMenu();
void drawMenu(const GameState& game, sf::RenderWindow& window);
void keyPressedMenu(GameState& game, const sf::Event::KeyPressed* event);
void exitMenu(GameState& game);

namespace GameStatuses {
	extern GameStatus menu;
};

#endif
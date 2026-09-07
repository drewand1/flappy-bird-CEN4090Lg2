#ifndef GAMEDRAW_H
#define GAMEDRAW_H

#include <SFML/Window.hpp>
#include "gamelogic.h"

void initGameDraw();
void drawGame(const GameState& game, sf::RenderWindow& window);

#endif
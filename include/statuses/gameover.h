#ifndef GAMEOVER_H
#define GAMEOVER_H

#include "gamelogic.h"

void mouseGameOver(GameState& game, const sf::Event::MouseButtonPressed*);
void keyGameOver(GameState& game, const sf::Event::KeyPressed*);
void drawGameOver(const GameState& game, sf::RenderWindow& window);

namespace GameStatuses {
	extern GameStatus gameOver;
}

#endif
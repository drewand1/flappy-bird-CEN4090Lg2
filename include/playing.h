#ifndef PLAYING_H
#define PLAYING_H

#include "gamelogic.h"

void keyPressedPlaying(GameState& game, const sf::Event::KeyPressed* event);
void drawPlaying(const GameState& game, sf::RenderWindow& window);

namespace GameStatuses {
	extern GameStatus playing;
}

#endif
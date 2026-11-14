#include "gameconfig.h"
#include "gamelogic.h"
#include "collision.h"
#include "gamestatuses.h"
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <SFML/Audio.hpp>

void resetGame(GameState& game) {
	setGameStatus(game, GameStatuses::menu);
	game.gameOverMusic.stop();

	// Check to restart menu music
if (game.menuMusic.getStatus() != sf::SoundSource::Status::Playing) {
    game.menuMusic.play(); 
}

	game.alive = true;
	game.health = 3;
	game.lastHitTime = sf::seconds(-1.0f);
	game.bird.pos = {0.0f, 0.0f};
	game.bird.yVel = 0.0f;
	game.pipes.clear();
	game.score = 0;
	game.scrollDistance = 0.0f;
}

void playGame(GameState& game) {
	setGameStatus(game, GameStatuses::playing);
	game.menuMusic.stop(); // Stop the menu music when the game starts
	game.lastPipeSpawn = game.clock.getElapsedTime(); // don't insta-spawn a pipe on start
	game.lastTick = game.clock.getElapsedTime(); // avoid a big dt jump on the first tick of gameplay
}

void setGameStatus(GameState& game, GameStatus status) {
	if (game.status.exit)
		game.status.exit(game);

	game.status = status;
	
	if (status.enter)
		status.enter(game);
}

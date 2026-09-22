#include "gameconfig.h"
#include "gamelogic.h"
#include "collision.h"
#include <cstdlib>
#include <algorithm>

void resetGame(GameState& game) {
	game.status = GameStatus::Menu;
	game.alive = true;
	game.bird.pos = {0.0f, 0.0f};
	game.bird.yVel = 0.0f;
	game.pipes.clear();
	game.score = 0;
}

void runTickLogic(GameState& game, sf::Window& window) {
	sf::Time now = game.clock.getElapsedTime();
	float dt = (now - game.lastTick).asSeconds();
	game.lastTick = now;
	
	sf::Vector2u winSz = window.getSize();
	float winTop = static_cast<float>(winSz.y) / 2.0f;
	float winBottom = -static_cast<float>(winSz.y) / 2.0f;
	float winLeft = -static_cast<float>(winSz.x) / 2.0f;
	
	// let dead bird fall to winBottom
	if (!game.alive) {
		if (game.bird.pos.y > winBottom + BIRD_RADIUS) {
			game.bird.pos.y += game.bird.yVel * dt;
			game.bird.yVel -= GRAVITY_CONST * dt;
			if (game.bird.pos.y <= winBottom + BIRD_RADIUS) {
				game.bird.pos.y = winBottom + BIRD_RADIUS;
				game.bird.yVel = 0.0f;
			}
		}
		return;
	}
	
	// Update bird
	game.bird.pos.x = winSz.x / -2.4f;
	game.bird.pos.y += game.bird.yVel * dt;
	game.bird.yVel -= GRAVITY_CONST * dt;
	
	// Update pipes
	for (auto& pipe : game.pipes) {
		pipe.xPos -= PIPE_BASE_SPEED * dt;
	}
	
	// despawn passed pipes (left of screen)
	std::erase_if(game.pipes, [winLeft](const Pipe& pipe) {
		return pipe.xPos + PIPE_HALF_WIDTH < winLeft;
	});
	
	// Spawn pipe
	if ((now - game.lastPipeSpawn).asSeconds() > PIPE_BASE_SPAWN_COOLDOWN) {
		Pipe pipe;
		pipe.gapHeight = PIPE_BASE_GAP_SIZE;
		pipe.gapPos = static_cast<float>(rand() % (static_cast<int>(0.8f * winSz.y)) - static_cast<int>(0.4f * winSz.y));
		pipe.xPos = winSz.x / 2.0f + PIPE_HALF_WIDTH;
		game.pipes.push_back(pipe);
		game.lastPipeSpawn = now;
	}
	
	// Collision detection
	if (checkCollisions(game, winSz)) {
		game.alive = false;
		if (game.bird.pos.y - BIRD_RADIUS <= winBottom) {
			game.bird.pos.y = winBottom + BIRD_RADIUS;
			game.bird.yVel = 0.0f;
		} else if (game.bird.pos.y + BIRD_RADIUS >= winTop) {
			game.bird.pos.y = winTop - BIRD_RADIUS;
			game.bird.yVel = 0.0f;
		}
	}
}

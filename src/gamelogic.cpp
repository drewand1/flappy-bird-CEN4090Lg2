#include <gameconfig.h>
#include <gamelogic.h>
#include <cstdlib>

void runTickLogic(GameState& game, sf::Window& window) {
	sf::Time now = game.clock.getElapsedTime();
	float dt = (now - game.lastTick).asSeconds();
	game.lastTick = now;
	
	sf::Vector2u winSz = window.getSize();
	float winTop = winSz.y / 2.0f;
	float winBottom = winSz.y / -2.0f;
	float winLeft = winSz.x / -2.0f;
	float winRight = winSz.x / 2.0f;
	
	// Update bird
	game.bird.pos.x = winSz.x / -2.4f;
	game.bird.pos.y = std::clamp(game.bird.pos.y + game.bird.yVel * dt, winBottom, winTop);
	game.bird.yVel -= GRAVITY_CONST * dt;
	
	// Update pipes
	for (int i = 0; i < game.pipes.size(); i++) {
		Pipe& pipe = game.pipes[i];
		pipe.xPos -= PIPE_BASE_SPEED * dt;
	}
	
	while (game.pipes.size() > 0 && game.pipes[0].xPos < winSz.x / -2.0f) {
		game.pipes.erase(game.pipes.begin());
	}
	
	// Spawn pipe
	if ((now - game.lastPipeSpawn).asSeconds() > PIPE_BASE_SPAWN_COOLDOWN) {
		Pipe pipe;
		pipe.gapHeight = PIPE_BASE_GAP_SIZE;
		pipe.gapPos = rand() % (static_cast<int>(0.8f * winSz.y)) - static_cast<int>(0.4f * winSz.y);
		pipe.xPos = winSz.x / 2.0f - 32.0f;
		game.pipes.push_back(pipe);
		game.lastPipeSpawn = now;
	}
}
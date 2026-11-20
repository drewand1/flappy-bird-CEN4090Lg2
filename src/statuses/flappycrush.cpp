#include <vector>
#include <cstdint>
#include <algorithm>
#include "gamestatuses.h"
#include "gamedraw.h"
#include "pipes.h"
#include "gameconfig.h"

constexpr float GRAV_CONST = -10.0f;
constexpr float DEFAULT_PIPE_OPENING = 100.0f;
constexpr float PIPE_X = 200.0f;
constexpr float BOUNCE_BACK = 100.0f;
constexpr float BOUNCE_BACK_DAMP_FACTOR = 0.5f;
constexpr float PIPE_MOVE_SPEED = 10.0f;

// Types
struct CrushBird {
	sf::Vector2f position;
	sf::Vector2f velocity;
	uint8_t jumpAtYMinus = 0;
	uint8_t skin = 0;
	uint8_t jumpPower = 0;
	uint8_t targetXVel = 0;
};

struct CrushGame {
	std::vector<CrushBird> birds;
	float pipeOpening = DEFAULT_PIPE_OPENING;
	bool shouldClosePipes = false;
	sf::Time lastBirdSpawnTick;
};

// Internal globals
static CrushGame globalCrush;

// Fn decs
void updateBird(GameState& game, CrushGame& crush, size_t idx, float dt);

void crushKeyPressed(GameState& game, const sf::Event::KeyPressed* event);
void crushDraw(const GameState& game, sf::RenderWindow& window);
void crushTick(GameState& game, sf::RenderWindow& window);
void crushEnter(GameState& game);
void crushExit(GameState& game);

// Defs
GameStatus GameStatuses::flappyCrush = {
	.keyPressed = crushKeyPressed,
	.draw = crushDraw,
	.tick = crushTick,
	.enter = crushEnter,
	.exit = crushExit
};

void updateBird(GameState& game, CrushGame& crush, size_t idx, float dt) {
	CrushBird& bird = crush.birds[idx];

	// Updating position and velocity
	bird.position += bird.velocity * dt;
	bird.velocity.y -= GRAV_CONST * dt;
	
	if (bird.targetXVel - bird.velocity.x > 0.01f) { // if bird is knocked back
		float diff = bird.targetXVel - bird.velocity.x;
		bird.velocity.x += diff * BOUNCE_BACK_DAMP_FACTOR * dt;

	}

	if (bird.position.y <= bird.jumpAtYMinus) {
		bird.velocity.y += bird.jumpPower;
	}

	// Collision handling
	float birdLeftX = bird.position.x - BIRD_RADIUS;
	float birdRightX = bird.position.x + BIRD_RADIUS;
	float pipeLeftX = PIPE_X - PIPE_HALF_WIDTH;
	float pipeRightX = PIPE_X + PIPE_HALF_WIDTH;
	bool inXBounds = birdRightX > pipeLeftX && birdLeftX < pipeRightX;
	if (inXBounds) {
		bool inYBounds = bird.position.y < crush.pipeOpening * 0.5f
			&& bird.position.y > crush.pipeOpening * -0.5f;
		if (inYBounds) {	// In the gap
			crush.birds[idx] = crush.birds[crush.birds.size() - 1];	// WARNING: Invalidates bird reference!
			crush.birds.pop_back();
			game.score++;

			return;
		} else {		// On the side
			float diff = birdRightX - pipeLeftX;
			bird.position.x -= diff;
			bird.velocity.x = -BOUNCE_BACK;
			game.hitSound.play();

			return;
		}
	}

	// Right edge detection
	if (birdRightX >= VIEW_WIDTH * 0.5f) {
		game.health--;
	}

}

void crushKeyPressed(GameState& game, const sf::Event::KeyPressed* event) {

}

void crushDraw(const GameState& game, sf::RenderWindow& window) {}

void crushTick(GameState& game, sf::RenderWindow& window) {
	sf::Time now = game.clock.getElapsedTime();
	float dt = (now - game.lastTick).asSeconds();
	game.lastTick = now;

	// Handling pipes
	bool spaceDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Space);

	if (spaceDown && globalCrush.pipeOpening > 0.05f) {
		globalCrush.pipeOpening -= PIPE_MOVE_SPEED * dt;
	}

	if (!spaceDown && (DEFAULT_PIPE_OPENING - globalCrush.pipeOpening > 0.05f)) {
		globalCrush.pipeOpening += PIPE_MOVE_SPEED * dt;
	}

	globalCrush.pipeOpening = std::clamp(globalCrush.pipeOpening, 0.0f, DEFAULT_PIPE_OPENING);

	// TODO: Handle bird spawning

	// Handling birds
	for (size_t i = 0; i < globalCrush.birds.size(); i++) {
		updateBird(game, globalCrush, i, dt);
	}

	// Handling game over
	if (game.health <= 0) {
		game.alive = false;
		setGameStatus(game, GameStatuses::gameOver);
		game.gameOverMusic.play();
	}

}

void crushEnter(GameState& game) {
	globalCrush.birds.reserve(100);
}

void crushExit(GameState& game) {
	globalCrush.birds.clear();
	globalCrush.birds.shrink_to_fit();
}
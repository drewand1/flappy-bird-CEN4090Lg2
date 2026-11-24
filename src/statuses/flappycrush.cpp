#include <cstdlib>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <random>
#include <iostream>
#include "gamestatuses.h"
#include "gameconfig.h"
#include "gamedraw.h"
#include "pipes.h"
#include "background.h"
#include "screencalc.h"

constexpr float GRAV_CONST = -200.0f;
constexpr float DEFAULT_PIPE_OPENING = 200.0f;
constexpr float PIPE_X = 200.0f;
constexpr float BOUNCE_BACK = 500.0f;
constexpr float BOUNCE_BACK_DAMP_FACTOR = 0.8f;
constexpr float PIPE_MOVE_SPEED = 1600.0f;
constexpr float MAX_BIRD_SPAWN_COOLDOWN = 3.0f;
constexpr float MIN_BIRD_SPAWN_COOLDOWN = 0.1f;

// Types
struct CrushBird {
	sf::Vector2f position = {-500, 0};
	sf::Vector2f velocity = {0, 0};
	int8_t jumpAtY = 0;
	uint8_t skin = 0;
	uint8_t jumpPower = 0;
	uint8_t targetXVel = 0;
};

struct CrushGame {
	std::vector<CrushBird> birds;
	float pipeOpening = DEFAULT_PIPE_OPENING;
	bool shouldClosePipes = false;
	sf::Time lastBirdSpawnTick;
	float spawnCooldown = MAX_BIRD_SPAWN_COOLDOWN;
};

// Internal globals
static CrushGame globalCrush;
static sf::Text scoreText(gameFont, "SCORE: 0", 20u);
static sf::Text livesText(gameFont, "LIVES: 3", 20u);


// Fn decs
CrushBird makeRandomBird();
void updateBird(GameState& game, CrushGame& crush, size_t& idx, float dt);

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

CrushBird makeRandomBird() {
	// Magic constants everywhere!
	CrushBird newBird;
	newBird.jumpAtY = static_cast<int8_t>((rand() % 120) - 120);
	newBird.targetXVel = static_cast<uint8_t>((rand() % 300) + 700);
	newBird.jumpPower = static_cast<uint8_t>((rand() % 100) + 100);
	newBird.skin = static_cast<uint8_t>(rand() % 8);
	newBird.velocity.x = newBird.targetXVel;
	return newBird;
}

void updateBird(GameState& game, CrushGame& crush, size_t& idx, float dt) {
	CrushBird& bird = crush.birds[idx];

	// Updating position and velocity
	bird.position += bird.velocity * dt;
	bird.velocity.y += GRAV_CONST * dt;
	
	if (bird.targetXVel - bird.velocity.x > 0.01f) { // if bird is knocked back
		float diff = bird.targetXVel - bird.velocity.x;
		bird.velocity.x += diff * BOUNCE_BACK_DAMP_FACTOR * dt;
	}

	if (bird.position.y < bird.jumpAtY) {
		bird.velocity.y = bird.jumpPower;
		bird.position.y = bird.jumpAtY;
	}

	// Collision handling (this is UGLY but I do not give a damn)
	float birdLeftX = bird.position.x - BIRD_RADIUS;
	float birdRightX = bird.position.x + BIRD_RADIUS;
	float pipeLeftX = PIPE_X - PIPE_HALF_WIDTH;
	float pipeRightX = PIPE_X + PIPE_HALF_WIDTH;
	float rightOverlap = birdRightX - pipeLeftX;
	float leftOverlap = pipeRightX - birdLeftX;
	float topOverlap = (bird.position.y + BIRD_RADIUS) - (crush.pipeOpening * 0.5f);
	float bottomOverlap = (crush.pipeOpening * -0.5f) - (bird.position.y - BIRD_RADIUS);
	bool inGap = bird.position.y + BIRD_RADIUS < crush.pipeOpening * 0.5f
			&& bird.position.y - BIRD_RADIUS > crush.pipeOpening * -0.5f;
	bool isCrushed;
	if (rightOverlap > 0 && leftOverlap > 0 && (topOverlap > 0 || bottomOverlap > 0)) {
		if (((topOverlap > 0 && topOverlap < rightOverlap) || (bottomOverlap > 0 && bottomOverlap < rightOverlap)) && globalCrush.pipeOpening > 0.1f) { // Crushed
			crush.birds[idx] = crush.birds[crush.birds.size() - 1];	// WARNING: Invalidates bird reference!
			crush.birds.pop_back();
			game.score++;
			scoreText.setString("SCORE: " + std::to_string(game.score));
			game.coinSound.play();
			idx--;
			return;
		} else {
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
		livesText.setString("LIVES: " + std::to_string(game.health));
		crush.birds[idx] = crush.birds[crush.birds.size() - 1];	// WARNING: Invalidates bird reference!
		crush.birds.pop_back();
		idx--;
	}
}

void crushKeyPressed(GameState& game, const sf::Event::KeyPressed* event) {
	switch (event->scancode) {
	case sf::Keyboard::Scancode::Escape:
		setGameStatus(game, GameStatuses::menu);
		break;
	}
}

void crushDraw(const GameState& game, sf::RenderWindow& window) {
	drawBackground(window, game.bgTheme, 0, 0);

	drawPipe(window, game.pipeSkin, normToScreen(sf::Vector2f(PIPE_X, globalCrush.pipeOpening * 0.5), window), false, 200);
	drawPipe(window, game.pipeSkin, normToScreen(sf::Vector2f(PIPE_X, globalCrush.pipeOpening * -0.5), window), true, 200);

	for (int i = 0; i < globalCrush.birds.size(); i++) {
		CrushBird& bird = globalCrush.birds[i];
		birdSprite.setTexture(birdSkinTexture(bird.skin));
		birdSprite.setPosition(normToScreen(bird.position, window));
		birdSprite.setRotation(sf::degrees(std::clamp(-40 - (bird.velocity.y - BIRD_JUMP_VEL) / 10.0f, -40.0f, 60.0f)));
		window.draw(birdSprite);
	}

	window.draw(scoreText);
	window.draw(livesText);
}

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

	// Spawn cooldown changing
	if (globalCrush.spawnCooldown > MIN_BIRD_SPAWN_COOLDOWN) {
		globalCrush.spawnCooldown -= (MAX_BIRD_SPAWN_COOLDOWN - MIN_BIRD_SPAWN_COOLDOWN) * 0.01 * dt;
	}

	// Handle bird spawning
	if ((now - globalCrush.lastBirdSpawnTick).asSeconds() >= globalCrush.spawnCooldown) {
		globalCrush.lastBirdSpawnTick = now;
		globalCrush.birds.emplace_back(makeRandomBird());
	}

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
	globalCrush.lastBirdSpawnTick = game.clock.getElapsedTime();
	globalCrush.shouldClosePipes = false;
	globalCrush.pipeOpening = DEFAULT_PIPE_OPENING;
	globalCrush.spawnCooldown = MAX_BIRD_SPAWN_COOLDOWN;

	scoreText.setString("SCORE: 0");
	scoreText.setPosition({10, 10});
	scoreText.setOutlineThickness(3);
	scoreText.setOutlineColor({14, 14, 14, 255});
	livesText.setString("LIVES: 3");
	livesText.setPosition({10, 36});
	livesText.setOutlineThickness(3);
	livesText.setOutlineColor({14, 14, 14, 255});
}

void crushExit(GameState& game) {
	globalCrush.birds.clear();
	globalCrush.birds.shrink_to_fit();

	// CANNOT call resetGame() because resetGame() sets status which invokes
	// this function again and thus we have infinite recursion.
	game.alive = true;
	game.health = 3;
	game.lastHitTime = sf::seconds(-1.0f);
	game.bird.pos = {0.0f, 0.0f};
	game.bird.yVel = 0.0f;
	game.pipes.clear();
	game.score = 0;
	game.scrollDistance = 0.0f;
}
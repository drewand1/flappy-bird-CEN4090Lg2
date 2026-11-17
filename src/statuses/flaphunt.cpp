#include <vector>
#include <SFML/Graphics.hpp>
#include "gameconfig.h"
#include "gamedraw.h"
#include "screencalc.h"
#include "background.h"
#include "gamestatuses.h"

struct HuntBird {
	sf::Vector2f pos;  // Centered world space, matching GameState::bird.pos
	sf::Vector2f vel;
	bool isShot = false;
};

void initFlapHunt(GameState& game);
void runFlapHuntTick(GameState& game, sf::RenderWindow& window);
void keyPressedFlapHunt(GameState& game, const sf::Event::KeyPressed* event);
void mousePressedFlapHunt(GameState& game, const sf::Event::MouseButtonPressed* event);
void drawFlapHunt(const GameState& game, sf::RenderWindow& window);

static std::vector<HuntBird> huntTargets;
static int ammoShells = 3;

GameStatus GameStatuses::flapHunt = {
	.keyPressed = keyPressedFlapHunt,
	.mouseButtonPressed = mousePressedFlapHunt,
	.draw = drawFlapHunt,
	.tick = runFlapHuntTick,
	.enter = initFlapHunt
};

void initFlapHunt(GameState& game) {
	huntTargets.clear();
	ammoShells = 3;
	game.score = 0;
	game.health = 3;

	// Spawn a starter target bird flying from left to right
	HuntBird target;
	target.pos = { -VIEW_WIDTH / 2.0f + 40.0f, 0.0f };
	target.vel = { 160.0f, 120.0f };
	huntTargets.push_back(target);
}

void keyPressedFlapHunt(GameState& game, const sf::Event::KeyPressed* event) {
	// Press ESC to return to the main menu
	if (event->scancode == sf::Keyboard::Scancode::Escape) {
		resetGame(game);
	}
}

void mousePressedFlapHunt(GameState& game, const sf::Event::MouseButtonPressed* event) {
	if (event->button != sf::Mouse::Button::Left || ammoShells <= 0)
		return;

	ammoShells--;
	game.flapSound.play(); // Placeholder gunshot sound

	// Map mouse click to view coordinates (same method used in birdselect.cpp)
	sf::Vector2f mouseViewPos = game.window->mapPixelToCoords(event->position);

	for (auto& target : huntTargets) {
		if (!target.isShot) {
			sf::Vector2f birdScreenPos = normToScreen(target.pos, *game.window);
			float dx = mouseViewPos.x - birdScreenPos.x;
			float dy = mouseViewPos.y - birdScreenPos.y;

			// Check if click is within the bird's clickable radius
			if ((dx * dx + dy * dy) <= (BIRD_RADIUS * 2.0f) * (BIRD_RADIUS * 2.0f)) {
				target.isShot = true;
				target.vel = { 0.0f, -250.0f }; // Shot bird drops straight down
				game.score++;
				game.coinSound.play();
				break;
			}
		}
	}
}

void runFlapHuntTick(GameState& game, sf::RenderWindow& window) {
	sf::Time now = game.clock.getElapsedTime();
	float dt = (now - game.lastTick).asSeconds();
	game.lastTick = now;

	for (auto& target : huntTargets) {
		target.pos.x += target.vel.x * dt;
		target.pos.y += target.vel.y * dt;

		if (target.isShot) {
			// Apply gravity so shot bird falls to the ground
			target.vel.y -= GRAVITY_CONST * dt;
		} else {
			// Simple bobbing/flapping AI to keep the target bird in the air
			if (target.pos.y < -40.0f) {
				target.vel.y = BIRD_JUMP_VEL * 0.8f;
			} else {
				target.vel.y -= (GRAVITY_CONST * 0.5f) * dt;
			}
		}
	}

	// Respawn a new bird & reload shells when the current bird leaves the screen
	if (!huntTargets.empty()) {
		const HuntBird& b = huntTargets.front();
		bool fellToGround = b.isShot && (b.pos.y < -VIEW_HEIGHT / 2.0f);
		bool escapedRight = !b.isShot && (b.pos.x > VIEW_WIDTH / 2.0f + BIRD_WIDTH);

		if (fellToGround || escapedRight) {
			if (escapedRight) {
				game.hitSound.play();
				if (game.health > 0) game.health--;
				if (game.health == 0) {
					game.alive = false;
					setGameStatus(game, GameStatuses::gameOver);
					game.gameOverMusic.play();
					return;
				}
			}
			huntTargets.clear();
			ammoShells = 3;
			HuntBird nextBird;
			nextBird.pos = { -VIEW_WIDTH / 2.0f + 40.0f, 0.0f };
			nextBird.vel = { 160.0f, 120.0f };
			huntTargets.push_back(nextBird);
		}
	}
}

void drawFlapHunt(const GameState& game, sf::RenderWindow& window) {
	drawBackground(window, game.bgTheme, 0.0f, 0.0f);

	// Draw target birds using the player's selected bird skin
	for (const auto& target : huntTargets) {
		birdSprite.setTexture(birdSkinTexture(game.birdSkin));
		birdSprite.setPosition(normToScreen(target.pos, window));
		birdSprite.setRotation(sf::degrees(target.isShot ? 90.0f : 0.0f));
		window.draw(birdSprite);
	}

	// Draw HUD (Score, Lives, and Shells Remaining)
	std::string uiString = "FLAP HUNT | Shells: " + std::to_string(ammoShells) +
	                       "\nScore: " + std::to_string(game.score) +
	                       " | Lives: " + std::to_string(game.health) +
	                       "\n[ESC] Menu";
	sf::Text uiText(gameFont, uiString, 14);
	uiText.setFillColor(sf::Color::White);
	uiText.setOutlineColor(sf::Color::Black);
	uiText.setOutlineThickness(2.0f);
	uiText.setPosition({ 20.0f, 20.0f });
	window.draw(uiText);
}
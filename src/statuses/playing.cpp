#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>
#include "gamedraw.h"
#include "screencalc.h"
#include "gameconfig.h"
#include "string"
#include "gamestatuses.h"
#include "background.h"
#include "pipes.h"
#include "collision.h"

using namespace std;

GameStatus GameStatuses::playing = {
	.keyPressed = keyPressedPlaying,
	.mouseButtonPressed = nullptr,
	.draw = drawPlaying,
	.tick = tickPlaying
};

void keyPressedPlaying(GameState& game, const sf::Event::KeyPressed* event) {
	if (game.alive){
		game.bird.yVel = BIRD_JUMP_VEL;
		game.flapSound.play(); // Plays the flap sound when the bird jumps
	}
}

void drawPlaying(const GameState& game, sf::RenderWindow& window) {
	// Draw background first so its behind everything
	drawBackground(window, game.bgTheme, game.scrollDistance, game.bird.pos.y);

	// Draw bird
	float timeSinceHit = (game.clock.getElapsedTime() - game.lastHitTime).asSeconds();
	if (!game.alive || timeSinceHit >= 1.0f || static_cast<int>(timeSinceHit * 10.0f) % 2 == 0) {
		birdSprite.setTexture(birdSkinTexture(game.birdSkin));
		birdSprite.setPosition(normToScreen(game.bird.pos, window));
		birdSprite.setRotation(sf::degrees(std::clamp(-40 - (game.bird.yVel - BIRD_JUMP_VEL) / 10.0f, -40.0f, 60.0f)));
		window.draw(birdSprite);
	}
		
	// Draw pipes
	for (int i = 0; i < game.pipes.size(); i++) {
		const Pipe& pipe = game.pipes[i];
		float gapTop = pipe.gapPos + pipe.gapHeight / 2.0f;
		float gapBottom = pipe.gapPos - pipe.gapHeight / 2.0f;

		// top one goes from the gap up to the top of the screen,
		// bottom one goes from the gap down to the ground (21 is the ground strip in background.cpp)
		drawPipe(window, game.pipeSkin, normToScreen({pipe.xPos, gapTop}, window), false, VIEW_HEIGHT / 2.0f - gapTop);
		drawPipe(window, game.pipeSkin, normToScreen({pipe.xPos, gapBottom}, window), true, gapBottom + VIEW_HEIGHT / 2.0f - 21.0f);
	}

	// Draw Score and Lives
	// TODO: make it so text only redraws when score or lives change; low pri
	std::string uiString = "Score: " + std::to_string(game.score) + "\nLives: " + std::to_string(game.health);
	sf::Text uiText(gameFont, uiString, 16);
	uiText.setFillColor(sf::Color::White);
	uiText.setOutlineColor(sf::Color::Black);
	uiText.setOutlineThickness(2.0f);

	// Calculate position dynamically based on the text's actual width
	sf::FloatRect textBounds = uiText.getLocalBounds();
	uiText.setPosition({ VIEW_WIDTH - textBounds.size.x - 20.0f, 20.0f });

	window.draw(uiText);
}

void tickPlaying(GameState& game, sf::RenderWindow& window) {
	sf::Time now = game.clock.getElapsedTime();
	float dt = (now - game.lastTick).asSeconds();
	game.lastTick = now;

	sf::Vector2u winSz = window.getSize();
	float winTop = static_cast<float>(winSz.y) / 2.0f;
	float winBottom = -static_cast<float>(winSz.y) / 2.0f;
	float winLeft = -static_cast<float>(winSz.x) / 2.0f;

	// let dead bird fall to winBottom
	if (!game.alive) {
		if (game.bird.pos.y > -VIEW_HEIGHT / 2 + BIRD_RADIUS) {
			game.bird.pos.y += game.bird.yVel * dt;
			game.bird.yVel -= GRAVITY_CONST * dt;
			if (game.bird.pos.y <= -VIEW_HEIGHT / 2 + BIRD_RADIUS) {
				game.bird.pos.y = -VIEW_HEIGHT / 2 + BIRD_RADIUS;
				game.bird.yVel = 0.0f;
			}
		}
		return;
	}

	// Update bird
	game.bird.pos.x = -VIEW_WIDTH / 2.0f + 80.0f;
	game.bird.pos.y += game.bird.yVel * dt;
	game.bird.yVel -= GRAVITY_CONST * dt;

	// Move background
	game.scrollDistance += PIPE_BASE_SPEED * dt;

	// Update pipes
	for (auto& pipe : game.pipes) {
		pipe.xPos -= PIPE_BASE_SPEED * dt;

		if (!pipe.passed && pipe.xPos + PIPE_HALF_WIDTH < game.bird.pos.x - BIRD_RADIUS) {
			pipe.passed = true;
			game.score++;
			game.coinSound.play(); // Play the coin sound when the bird passes a pipe
		}
	}

	// despawn passed pipes (left of screen)
	std::erase_if(game.pipes, [winLeft](const Pipe& pipe) {
		return pipe.xPos + PIPE_HALF_WIDTH < -VIEW_WIDTH / 2.0f;
		});

	// Spawn pipe
	if ((now - game.lastPipeSpawn).asSeconds() > PIPE_BASE_SPAWN_COOLDOWN) {
		Pipe pipe;
		pipe.gapHeight = PIPE_BASE_GAP_SIZE;
		pipe.gapPos = static_cast<float>(rand() % static_cast<int>(VIEW_HEIGHT * 0.8f) - static_cast<int>(0.4f * VIEW_HEIGHT));
		pipe.xPos = VIEW_WIDTH / 2.0f + PIPE_HALF_WIDTH;
		game.pipes.push_back(pipe);
		game.lastPipeSpawn = now;
	}

	// Collision detection
	if (checkCollisions(game, sf::Vector2u(VIEW_WIDTH, VIEW_HEIGHT))) {
		if (game.bird.pos.y - BIRD_RADIUS <= -VIEW_HEIGHT / 2.0f) {
			game.bird.pos.y = -VIEW_HEIGHT / 2.0f + BIRD_RADIUS;
			game.bird.yVel = 0.0f;
		}
		else if (game.bird.pos.y + BIRD_RADIUS >= VIEW_HEIGHT / 2.0f) {
			game.bird.pos.y = VIEW_HEIGHT / 2.0f - BIRD_RADIUS;
			game.bird.yVel = 0.0f;
		}

		for (auto const& pipe : game.pipes) {
			if (checkPipeCollision(game.bird, pipe)) {
				float upperPipeBottom = pipe.gapPos + pipe.gapHeight / 2.0f;
				float lowerPipeTop = pipe.gapPos - pipe.gapHeight / 2.0f;
				if (game.bird.pos.y - BIRD_RADIUS <= lowerPipeTop) {
					game.bird.pos.y = lowerPipeTop + BIRD_RADIUS;
					game.bird.yVel = 0.0f;
				}
				else if (game.bird.pos.y + BIRD_RADIUS >= upperPipeBottom) {
					game.bird.pos.y = upperPipeBottom - BIRD_RADIUS;
					game.bird.yVel = 0.0f;
				}
			}
		}

		// 1-second Invulnerability Cooldown
		if ((now - game.lastHitTime).asSeconds() > 1.0f) {
			game.health--;
			game.lastHitTime = now;
			game.hitSound.play(); // Play the hit sound when the bird collides with a pipe

			if (game.health <= 0) {
				game.alive = false;
				setGameStatus(game, GameStatuses::gameOver);
				game.gameOverMusic.play(); // Play the game over music
			}
		}
	}
}

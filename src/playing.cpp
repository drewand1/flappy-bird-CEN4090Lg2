#include <SFML/Graphics.hpp>
#include <algorithm>
#include "gamedraw.h"
#include "screencalc.h"
#include "gameconfig.h"
#include "string"
#include "playing.h"
#include "background.h"
#include "pipes.h"
#include "birdselect.h"
#include <iostream>
using namespace std;

GameStatus GameStatuses::playing = {
	.keyPressed = keyPressedPlaying,
	.mouseButtonPressed = nullptr,
	.draw = drawPlaying
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

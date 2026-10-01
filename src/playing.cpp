#include <SFML/Graphics.hpp>
#include <algorithm>
#include "gamedraw.h"
#include "screencalc.h"
#include "gameconfig.h"
#include "string"
#include "playing.h"

GameStatus GameStatuses::playing = {
	.keyPressed = keyPressedPlaying,
	.mouseButtonPressed = nullptr,
	.draw = drawPlaying
};

void keyPressedPlaying(GameState& game, const sf::Event::KeyPressed* event) {
	if (game.alive)
		game.bird.yVel = BIRD_JUMP_VEL;
}

void drawPlaying(const GameState& game, sf::RenderWindow& window) {
	// Draw bird
	birdSprite.setPosition(normToScreen(game.bird.pos, window));
	birdSprite.setRotation(sf::degrees(std::clamp(-40 - (game.bird.yVel - BIRD_JUMP_VEL) / 10.0f, -40.0f, 60.0f)));
	window.draw(birdSprite);	
		
	// Draw pipes
	for (int i = 0; i < game.pipes.size(); i++) {
		const Pipe& pipe = game.pipes[i];
		pipeSprite.setRotation(sf::degrees(0));
		pipeSprite.setPosition(normToScreen({pipe.xPos, pipe.gapPos + pipe.gapHeight / 2.0f}, window));
		window.draw(pipeSprite);
		pipeSprite.setRotation(sf::degrees(180));
		pipeSprite.setPosition(normToScreen({pipe.xPos, pipe.gapPos - pipe.gapHeight / 2.0f}, window));
		window.draw(pipeSprite);
	}

	// Draw Score and Lives
	std::string uiString = "Score: " + std::to_string(game.score) + "\nLives: " + std::to_string(game.health);
	sf::Text uiText(gameFont, uiString, 16);
	uiText.setFillColor(sf::Color::White);
	uiText.setOutlineColor(sf::Color::Black);
	uiText.setOutlineThickness(2.0f);

	// Calculate position dynamically based on the text's actual width
	sf::FloatRect textBounds = uiText.getLocalBounds();
	uiText.setPosition({ window.getSize().x - textBounds.size.x - 20.0f, 20.0f });

	window.draw(uiText);
}

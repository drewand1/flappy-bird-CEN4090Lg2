#include <SFML/Graphics.hpp>
#include <algorithm>
#include "gamedraw.h"
#include "screencalc.h"
#include "gameconfig.h"
#include "string"

static sf::Font gameFont("rsc/fonts/PressStart2P-Regular.ttf");
static sf::Texture birdTex("rsc/textures/bird.png");
static sf::Texture pipeTex("rsc/textures/pipe.png");
static sf::Sprite birdSprite(birdTex);
static sf::Sprite pipeSprite(pipeTex);

void initGameDraw() {
	birdTex.setSmooth(false);
	pipeTex.setSmooth(false);
	
	birdSprite.setTexture(birdTex);
	birdSprite.setScale({2.0f, 2.0f});
	birdSprite.setOrigin({8.0f, 8.0f});
	
	pipeSprite.setTexture(pipeTex);
	pipeSprite.setScale({4.0f, 10.0f});
	pipeSprite.setOrigin({8.0f, static_cast<float>(pipeSprite.getLocalBounds().size.y)}); // So that we can rotate around the mouth of the pipe to make it easier to position
}

void drawGame(const GameState& game, sf::RenderWindow& window) {
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

void drawGameOver(const GameState& game, sf::RenderWindow& window) {
	drawGame(game, window);

	sf::Vector2f winSz(window.getSize());

	sf::RectangleShape overlay(winSz);
	overlay.setFillColor(sf::Color(0, 0, 0, 150));
	window.draw(overlay);

	// "Game Over" Header Text
	sf::Text overText(gameFont, "GAME OVER", 32);
	overText.setFillColor(sf::Color::Red);
	overText.setOutlineColor(sf::Color::White);
	overText.setOutlineThickness(2.0f);
	sf::FloatRect overBounds = overText.getLocalBounds();
	overText.setOrigin({ overBounds.position.x + overBounds.size.x / 2.0f, overBounds.position.y + overBounds.size.y / 2.0f });
	overText.setPosition({ winSz.x / 2.0f, winSz.y * 0.2f });
	window.draw(overText);

	// Play Again Button
	sf::RectangleShape playBtn({ 200.0f, 50.0f });
	playBtn.setPosition({ winSz.x / 2.0f - 100.0f, winSz.y / 2.0f - 65.0f });
	playBtn.setFillColor(sf::Color(50, 150, 50));
	playBtn.setOutlineColor(sf::Color::White);
	playBtn.setOutlineThickness(2.0f);
	window.draw(playBtn);

	sf::Text playText(gameFont, "Play Again", 16);
	playText.setFillColor(sf::Color::White);
	sf::FloatRect playTBounds = playText.getLocalBounds();
	playText.setOrigin({ playTBounds.position.x + playTBounds.size.x / 2.0f, playTBounds.position.y + playTBounds.size.y / 2.0f });
	playText.setPosition({ winSz.x / 2.0f, winSz.y / 2.0f - 40.0f });
	window.draw(playText);

	// Main Menu Button
	sf::RectangleShape menuBtn({ 200.0f, 50.0f });
	menuBtn.setPosition({ winSz.x / 2.0f - 100.0f, winSz.y / 2.0f + 15.0f });
	menuBtn.setFillColor(sf::Color(150, 50, 50));
	menuBtn.setOutlineColor(sf::Color::White);
	menuBtn.setOutlineThickness(2.0f);
	window.draw(menuBtn);

	sf::Text menuText(gameFont, "Main Menu", 16);
	menuText.setFillColor(sf::Color::White);
	sf::FloatRect menuTBounds = menuText.getLocalBounds();
	menuText.setOrigin({ menuTBounds.position.x + menuTBounds.size.x / 2.0f, menuTBounds.position.y + menuTBounds.size.y / 2.0f });
	menuText.setPosition({ winSz.x / 2.0f, winSz.y / 2.0f + 40.0f });
	window.draw(menuText);
}
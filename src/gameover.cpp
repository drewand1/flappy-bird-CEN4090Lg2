#include <SFML/Graphics.hpp>
#include <algorithm>
#include "gamedraw.h"
#include "screencalc.h"
#include "gameconfig.h"
#include "string"
#include "playing.h"
#include "gameover.h"

GameStatus GameStatuses::gameOver = {
	.keyPressed = keyGameOver,
	.mouseButtonPressed = mouseGameOver,
	.draw = drawGameOver
};

void mouseGameOver(GameState& game, const sf::Event::MouseButtonPressed* event) {
	if (event->button == sf::Mouse::Button::Left) {
		sf::Vector2f mousePos = game.window->mapPixelToCoords(sf::Mouse::getPosition(*(game.window)));
		sf::Vector2f winSz(game.window->getSize());

		// Define bounds for buttons
		sf::FloatRect playAgainBounds({ winSz.x / 2.0f - 100.0f, winSz.y / 2.0f - 65.0f }, { 200.0f, 50.0f });
		sf::FloatRect menuBounds({ winSz.x / 2.0f - 100.0f, winSz.y / 2.0f + 15.0f }, { 200.0f, 50.0f });

		if (playAgainBounds.contains(mousePos)) {
			resetGame(game);
			playGame(game);
		}
		else if (menuBounds.contains(mousePos)) {
			resetGame(game);
		}
	}
}

void keyGameOver(GameState& game, const sf::Event::KeyPressed* event) {
	if (event->scancode == sf::Keyboard::Scan::Space) {
		resetGame(game);
		playGame(game);
	}
}

void drawGameOver(const GameState& game, sf::RenderWindow& window) {
	drawPlaying(game, window);

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
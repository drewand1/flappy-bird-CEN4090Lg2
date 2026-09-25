/*

Basic flappy bird skeleton

To do:
[]	Menu, score UI
[]	Score keeping
[x]	Collision detection (floor & pipe)
[]	Game over state
[]	Sounds
[]	Parallax scrolling effect

Known issues:
- Pipe gap position is determined by window height. If window is resized to be
	shorter, a pipe currently on screen might become unpassable because its
	position was determined based on the previous screen height. Frankly low
	priority bc who tf downsizes their window in the middle of flappy bird?

*/

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "gameconfig.h"
#include "gamelogic.h"
#include "gamedraw.h"
#include "screencalc.h"
#include "menu.h"

int main() {
	srand(time(NULL));
	initGameDraw();
	initMenu();

	sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "RAAAAAAH");
	window.setVerticalSyncEnabled(true);
	window.setKeyRepeatEnabled(false);

	GameState game;

	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {

			if (event->is<sf::Event::Closed>())
				window.close();

			if (const auto* resized = event->getIf<sf::Event::Resized>()) {
				sf::FloatRect view({ 0, 0 }, sf::Vector2f(resized->size));
				window.setView(sf::View(view));
			}

			// Keyboard Events
			if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
				if (keyPressed->scancode == sf::Keyboard::Scan::Space) {
					if (game.status == GameStatus::Menu) {
						game.status = GameStatus::Playing;
						game.lastPipeSpawn = game.clock.getElapsedTime(); // don't insta-spawn a pipe on start
						game.lastTick = game.clock.getElapsedTime(); // avoid a big dt jump on the first tick of gameplay
					}
					else if (game.status == GameStatus::Playing && game.alive) {
						game.bird.yVel = BIRD_JUMP_VEL;
					}
					else if (game.status == GameStatus::GameOver) {
						resetGame(game);
						game.status = GameStatus::Playing;
						game.lastTick = game.clock.getElapsedTime();
					}
				}
			}

			// Mouse Events
			if (const auto* mouseClick = event->getIf<sf::Event::MouseButtonPressed>()) {
				if (mouseClick->button == sf::Mouse::Button::Left && game.status == GameStatus::GameOver) {
					sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
					sf::Vector2f winSz(window.getSize());

					// Define bounds for buttons
					sf::FloatRect playAgainBounds({ winSz.x / 2.0f - 100.0f, winSz.y / 2.0f - 65.0f }, { 200.0f, 50.0f });
					sf::FloatRect menuBounds({ winSz.x / 2.0f - 100.0f, winSz.y / 2.0f + 15.0f }, { 200.0f, 50.0f });

					if (playAgainBounds.contains(mousePos)) {
						resetGame(game);
						game.status = GameStatus::Playing;
						game.lastTick = game.clock.getElapsedTime();
					}
					else if (menuBounds.contains(mousePos)) {
						resetGame(game);
					}
				}
			}
		}

		if (game.status == GameStatus::Playing) {
			runTickLogic(game, window);
		}

		window.clear();
		if (game.status == GameStatus::Menu) {
			drawMenu(game, window);
		}
		else if (game.status == GameStatus::Playing) {
			drawGame(game, window);
		}
		else if (game.status == GameStatus::GameOver) {
			drawGameOver(game, window);
		}
		window.display();

	}

	return 0;
}
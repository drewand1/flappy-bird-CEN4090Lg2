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
#include "playing.h"

int main() {
	srand(time(NULL));
	initGameDraw();
	initMenu();

	sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "RAAAAAAH");
	window.setVerticalSyncEnabled(true);
	window.setKeyRepeatEnabled(false);

	GameState game;
	game.window = &window;
	resetGame(game);

	setLetterboxView(window);

	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {

			if (event->is<sf::Event::Closed>())
				window.close();

			if (const auto* resized = event->getIf<sf::Event::Resized>()) {
				setLetterboxView(window);
			}

			// Keyboard Events
			if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
				if (game.status.keyPressed)
					game.status.keyPressed(game, keyPressed);
			}

			// Mouse Events
			if (const auto* mouseClick = event->getIf<sf::Event::MouseButtonPressed>()) {
				if (game.status.mouseButtonPressed)
					game.status.mouseButtonPressed(game, mouseClick);
			}
		}

		if (game.status.draw == drawPlaying) { // Disgustingly hacky but if it works it works
			runTickLogic(game, window);
		}

		window.clear();
		if (game.status.draw)
			game.status.draw(game, window);
		window.display();
	}

	return 0;
}
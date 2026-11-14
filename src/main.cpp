#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cenui.h>
#include "gameconfig.h"
#include "gamelogic.h"
#include "gamedraw.h"
#include "screencalc.h"
#include "gamestatuses.h"
#include "background.h"
#include "pipes.h"

int main() {
	srand(time(NULL));
	initGameDraw();
	initBackground();
	initPipes();
	initMenu();

	sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "RAAAAAAH");
	window.setVerticalSyncEnabled(true);
	window.setKeyRepeatEnabled(false);

	GameState game;
	game.window = &window;

	// Load audio files 
	if (game.flapBuffer.loadFromFile("rsc/audio/flap.wav")) {
    	game.flapSound.setBuffer(game.flapBuffer);
	}
	if (game.hitBuffer.loadFromFile("rsc/audio/hitHurt.wav")) {
   		game.hitSound.setBuffer(game.hitBuffer);
	}
	if (game.coinBuffer.loadFromFile("rsc/audio/pickupCoin.wav")) {
    	game.coinSound.setBuffer(game.coinBuffer);
	}
	if (game.menuMusic.openFromFile("rsc/audio/mainMenu.wav")) {
    	game.menuMusic.setLooping(true);
	}
	if (game.gameOverMusic.openFromFile("rsc/audio/gameover.wav")) {
		game.gameOverMusic.setLooping(false);
	}

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
				game.ui.handleMouseDown(window);
				if (game.status.mouseButtonPressed)
					game.status.mouseButtonPressed(game, mouseClick);
			}

			if (const auto* mouseClick = event->getIf<sf::Event::MouseButtonReleased>()) {
				game.ui.handleMouseUp(window);
			}
		}

		if (game.status.tick) { // TODO: Find a way to tell whether is a playing state
			game.status.tick(game, window);
		}

		window.clear();
		if (game.status.draw)
			game.status.draw(game, window);

		// draw ui
		window.display();
	}

	return 0;
}
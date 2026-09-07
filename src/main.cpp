/*

Basic flappy bird skeleton

To do:
[]	Menu, score UI
[]	Score keeping
[]	Collision detection (floor & pipe)
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

int main() {
	srand(time(NULL));
	initGameDraw();
	
	sf::RenderWindow window(sf::VideoMode({640, 480}), "RAAAAAAH");
	window.setVerticalSyncEnabled(true);
	window.setKeyRepeatEnabled(false);
	
	GameState game;
	
	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {

			if (event->is<sf::Event::Closed>())
				window.close();
				
			if (const auto* resized = event->getIf<sf::Event::Resized>()) {
				sf::FloatRect view({0, 0}, sf::Vector2f(resized->size));
				window.setView(sf::View(view));
			}
			
			if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
				if (keyPressed->scancode == sf::Keyboard::Scan::Space) {
					game.bird.yVel = BIRD_JUMP_VEL;
				}
			}
		}
		
		runTickLogic(game, window);
		
		window.clear();
		drawGame(game, window);
		window.display();
		
		
	}

	return 0;
}


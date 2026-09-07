#ifndef SCREENCALC_H
#define SCREENCALC_H

#include <SFML/Window.hpp>

sf::Vector2f normToScreen(sf::Vector2f v, sf::Window& window);
sf::Vector2f screenToNorm(sf::Vector2f v, sf::Window& window);

#endif
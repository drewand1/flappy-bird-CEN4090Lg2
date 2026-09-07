#include "screencalc.h"

sf::Vector2f normToScreen(sf::Vector2f v, sf::Window& window) {
	sf::Vector2u winSz = window.getSize();
	return {winSz.x / 2.0f + v.x, winSz.y / 2.0f - v.y};
}

sf::Vector2f screenToNorm(sf::Vector2f v, sf::Window& window) {
	sf::Vector2u winSz = window.getSize();
	return {v.x - winSz.x / 2.0f, winSz.y / 2.0f - v.y};
};
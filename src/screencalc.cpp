#include <SFML/Graphics.hpp>
#include "screencalc.h"
#include "gameconfig.h"

sf::Vector2f normToScreen(sf::Vector2f v, sf::Window& window) {
	return {VIEW_WIDTH / 2.0f + v.x, VIEW_HEIGHT / 2.0f - v.y};
}

sf::Vector2f screenToNorm(sf::Vector2f v, sf::Window& window) {
	sf::Vector2u winSz = window.getSize();
	return {v.x - VIEW_WIDTH / 2.0f, VIEW_HEIGHT / 2.0f - v.y};
}

void setLetterboxView(sf::RenderWindow& window) {
	sf::FloatRect viewport;
	sf::Vector2u winSz = window.getSize();
	float aspectRatio = static_cast<float>(winSz.x) / winSz.y;

	if (aspectRatio > (VIEW_WIDTH / VIEW_HEIGHT)) {
		float factor = 2 / aspectRatio; // fraction of height/width, * 2
		viewport = sf::FloatRect(sf::Vector2((1 - factor) * 0.5f, 0.0f), sf::Vector2(factor, 1.0f));
	} else {
		float factor = aspectRatio * 0.5f; // Fraction of width / height, / 2.
		viewport = sf::FloatRect(sf::Vector2(0.0f, (1 - factor) * 0.5f), sf::Vector2(1.0f, factor));
	}

	sf::View view(sf::FloatRect({0, 0}, {VIEW_WIDTH, VIEW_HEIGHT}));
	view.setViewport(viewport);
	window.setView(view);
}
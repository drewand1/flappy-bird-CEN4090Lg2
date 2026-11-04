#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SFML/Graphics.hpp>
#include <string>

constexpr int THEME_COUNT = 4;

struct Theme {
	std::string name;
	std::string folder; 
	float scale;
	sf::Texture sky;
	sf::Texture tex[4];
	bool has[4] = {};
	sf::Color under[4];
};

extern Theme themes[THEME_COUNT];

void initBackground();
void drawBackground(sf::RenderWindow& window, int theme, float scrollDistance, float cameraHeight);

#endif
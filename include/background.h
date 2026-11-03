#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SFML/Graphics.hpp>
#include <string>

constexpr int THEME_COUNT = 4; // bump if you add a theme in background.cpp

struct Theme {
	std::string name;
	std::string folder; // in rsc/textures/backgrounds/
	float scale;        // 3 for pixel art, around 1 for hi-res
	sf::Texture sky;
	sf::Texture tex[4]; // back, middle, front, ground
	bool has[4] = {};
	sf::Color under[4];
};

extern Theme themes[THEME_COUNT];

void initBackground();
void drawBackground(sf::RenderWindow& window, int theme, float scrollDistance, float cameraHeight);

#endif
#ifndef PIPES_H
#define PIPES_H

#include <SFML/Graphics.hpp>
#include <string>

constexpr int PIPE_COUNT = 4;
struct PipeSkin {
	std::string name;
	std::string file;
	float stretch;
	bool fit;
	float width;
	int tip;
	int hilt;
	sf::Texture tex;
	sf::IntRect rect;
};

extern PipeSkin pipeSkins[PIPE_COUNT];

void initPipes();
void setPipeSkin(sf::Sprite& s, int skin, float width);

void drawPipe(sf::RenderWindow& window, int skin, sf::Vector2f mouth, bool flip, float length);

#endif

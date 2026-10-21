#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SFML/Graphics.hpp>

void initBackground();

// cameraHeight is the bird's y, used for the vertical sway
void drawBackground(sf::RenderWindow& window, float scrollDistance, float cameraHeight);

#endif

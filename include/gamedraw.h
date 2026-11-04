#ifndef GAMEDRAW_H
#define GAMEDRAW_H

#include <SFML/Window.hpp>
#include "gamelogic.h"

extern sf::Font gameFont;
extern sf::Texture birdTex;
extern sf::Sprite birdSprite;

void initGameDraw();

#endif
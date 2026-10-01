#ifndef GAMEDRAW_H
#define GAMEDRAW_H

#include <SFML/Window.hpp>
#include "gamelogic.h"

extern sf::Font gameFont;
extern sf::Texture birdTex;
extern sf::Texture pipeTex;
extern sf::Sprite birdSprite;
extern sf::Sprite pipeSprite;

void initGameDraw();

#endif
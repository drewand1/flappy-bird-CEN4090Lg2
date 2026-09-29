#include <SFML/Graphics.hpp>
#include <algorithm>
#include "gamedraw.h"
#include "screencalc.h"
#include "gameconfig.h"
#include "string"

sf::Font gameFont("rsc/fonts/PressStart2P-Regular.ttf");
sf::Texture birdTex("rsc/textures/bird.png");
sf::Texture pipeTex("rsc/textures/pipe.png");
sf::Sprite birdSprite(birdTex);
sf::Sprite pipeSprite(pipeTex);

void initGameDraw() {
	birdTex.setSmooth(false);
	pipeTex.setSmooth(false);
	
	birdSprite.setTexture(birdTex);
	birdSprite.setScale({2.0f, 2.0f});
	birdSprite.setOrigin({8.0f, 8.0f});
	
	pipeSprite.setTexture(pipeTex);
	pipeSprite.setScale({4.0f, 10.0f});
	pipeSprite.setOrigin({8.0f, static_cast<float>(pipeSprite.getLocalBounds().size.y)}); // So that we can rotate around the mouth of the pipe to make it easier to position
}
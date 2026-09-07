#include <SFML/Graphics.hpp>
#include <algorithm>
#include "gamedraw.h"
#include "screencalc.h"
#include "gameconfig.h"

static sf::Texture birdTex("rsc/textures/bird.png");
static sf::Texture pipeTex("rsc/textures/pipe.png");
static sf::Sprite birdSprite(birdTex);
static sf::Sprite pipeSprite(pipeTex);

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

void drawGame(const GameState& game, sf::RenderWindow& window) {
	// Draw bird
	birdSprite.setPosition(normToScreen(game.bird.pos, window));
	birdSprite.setRotation(sf::degrees(std::clamp(-40 - (game.bird.yVel - BIRD_JUMP_VEL) / 10.0f, -40.0f, 60.0f)));
	window.draw(birdSprite);	
		
	// Draw pipes
	for (int i = 0; i < game.pipes.size(); i++) {
		const Pipe& pipe = game.pipes[i];
		pipeSprite.setRotation(sf::degrees(0));
		pipeSprite.setPosition(normToScreen({pipe.xPos, pipe.gapPos + pipe.gapHeight / 2.0f}, window));
		window.draw(pipeSprite);
		pipeSprite.setRotation(sf::degrees(180));
		pipeSprite.setPosition(normToScreen({pipe.xPos, pipe.gapPos - pipe.gapHeight / 2.0f}, window));
		window.draw(pipeSprite);
	}
}
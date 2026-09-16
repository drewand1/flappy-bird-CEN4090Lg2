#include <cmath>
#include "menu.h"

// Loads its own copies of the bird/pipe textures so this file doesn't
// depend on (or need changes to) gamedraw.cpp. Once we have a proper
// asset-loading system these can be pointed at shared textures instead.
static sf::Texture menuBirdTex("rsc/textures/bird.png");
static sf::Texture menuPipeTex("rsc/textures/pipe.png");
static sf::Texture menuGroundTex("rsc/textures/ground.png");
static sf::Sprite menuBirdSprite(menuBirdTex);
static sf::Sprite menuPipeSprite(menuPipeTex);
static sf::Sprite menuGroundSprite(menuGroundTex);
static sf::Font menuFont("rsc/fonts/PressStart2P-Regular.ttf");

void initMenu() {
	menuBirdTex.setSmooth(false);
	menuPipeTex.setSmooth(false);
	menuGroundTex.setSmooth(false);
	menuGroundTex.setRepeated(true);

	menuBirdSprite.setTexture(menuBirdTex);
	menuBirdSprite.setScale({2.0f, 2.0f});
	menuBirdSprite.setOrigin({8.0f, 8.0f});

	menuPipeSprite.setTexture(menuPipeTex);
	menuPipeSprite.setScale({4.0f, 10.0f});
	menuPipeSprite.setOrigin({8.0f, static_cast<float>(menuPipeSprite.getLocalBounds().size.y)});

	menuGroundSprite.setTexture(menuGroundTex);
}

// Main menu screen.
void drawMenu(const GameState& game, sf::RenderWindow& window) {
	sf::Vector2u winSz = window.getSize();
	float t = game.clock.getElapsedTime().asSeconds();

	// Sky background
	sf::RectangleShape sky(sf::Vector2f{winSz});
	sky.setPosition({0.f, 0.f});
	sky.setFillColor(sf::Color(78, 192, 202));
	window.draw(sky);

	// Ground strip - tiled grass/dirt texture, repeated across the window width.
	// Scale controls how "chunky" the pixel art looks (matches the general
	// weight of the bird/pipe sprites, which are also scaled up from 16px).
	float groundScale = 3.0f;
	float groundHeight = 16.0f * groundScale;

	// Solid backing fill first: the tile art has small transparent notches
	// along its jagged silhouette that don't perfectly line up where the
	// repeat wraps, which reads as visible seams. A solid dirt-colored
	// rectangle underneath means any gap shows matching dirt color instead
	// of whatever's behind it.
	sf::RectangleShape groundBacking({static_cast<float>(winSz.x), groundHeight});
	groundBacking.setPosition({0.f, winSz.y - groundHeight});
	groundBacking.setFillColor(sf::Color(122, 68, 74));
	window.draw(groundBacking);

	unsigned int groundRepeatWidth = static_cast<unsigned int>(std::ceil(winSz.x / groundScale));
	menuGroundSprite.setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(groundRepeatWidth), 16}));
	menuGroundSprite.setScale({groundScale, groundScale});
	menuGroundSprite.setPosition({0.f, winSz.y - groundHeight});
	window.draw(menuGroundSprite);

	// Decorative pipes flanking the menu. Rotated 180° so the wide cap
	// faces up (the source texture has its cap at the bottom, anchored
	// there since that's how gamedraw.cpp positions gameplay pipes).
	// Height is capped at half the window so there's clear open sky
	// above them -- handy since the first real gameplay pipe can start
	// roughly where these leave off once we transition out of the menu.
	float groundTopY = winSz.y - groundHeight;
	float pipeVisibleHeight = winSz.y * 0.5f;
	float pipeTexHeight = static_cast<float>(menuPipeTex.getSize().y);
	float pipeScaleY = pipeVisibleHeight / pipeTexHeight;

	menuPipeSprite.setScale({4.0f, pipeScaleY});
	menuPipeSprite.setRotation(sf::degrees(180));

	menuPipeSprite.setPosition({winSz.x * 0.15f, groundTopY - pipeVisibleHeight});
	window.draw(menuPipeSprite);

	menuPipeSprite.setPosition({winSz.x * 0.85f, groundTopY - pipeVisibleHeight});
	window.draw(menuPipeSprite);

	// Bobbing bird, front and center
	float bob = std::sin(t * 2.5f) * 10.0f;
	menuBirdSprite.setRotation(sf::degrees(0));
	menuBirdSprite.setPosition({winSz.x / 2.0f, winSz.y * 0.38f + bob});
	window.draw(menuBirdSprite);

	// Title, centered where the placeholder box used to be
	sf::Text title(menuFont, "Flappy Bird", static_cast<unsigned int>(winSz.y * 0.085f));
	title.setFillColor(sf::Color::White);
	title.setOutlineColor(sf::Color::Black);
	title.setOutlineThickness(3.f);
	sf::FloatRect titleBounds = title.getLocalBounds();
	title.setOrigin(titleBounds.position + titleBounds.size / 2.0f);
	title.setPosition({winSz.x / 2.0f, winSz.y * 0.15f});
	window.draw(title);

	// Pulsing "Press Space to Start" prompt
	float pulse = (std::sin(t * 4.0f) + 1.0f) / 2.0f;
	sf::Text prompt(menuFont, "Press Space to Start", static_cast<unsigned int>(winSz.y * 0.032f));
	prompt.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(120 + pulse * 135)));
	prompt.setOutlineColor(sf::Color::Black);
	prompt.setOutlineThickness(2.f);
	sf::FloatRect promptBounds = prompt.getLocalBounds();
	prompt.setOrigin(promptBounds.position + promptBounds.size / 2.0f);
	prompt.setPosition({winSz.x / 2.0f, winSz.y * 0.68f});
	window.draw(prompt);
}
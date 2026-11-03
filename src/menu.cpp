#include <cmath>
#include "gameconfig.h"
#include "menu.h"
#include "background.h"

// Loads its own copies of the bird/pipe textures so this file doesn't
// depend on (or need changes to) gamedraw.cpp. Once we have a proper
// asset-loading system these can be pointed at shared textures instead.
static sf::Texture menuBirdTex("rsc/textures/bird.png");
static sf::Texture menuPipeTex("rsc/textures/pipe.png");
static sf::Sprite menuBirdSprite(menuBirdTex);
static sf::Sprite menuPipeSprite(menuPipeTex);
static sf::Font menuFont("rsc/fonts/PressStart2P-Regular.ttf");

// Defining the game status
GameStatus GameStatuses::menu = {
	.keyPressed = keyPressedMenu,
	.mouseButtonPressed = nullptr,
	.draw = drawMenu
};

// Fn defs

void initMenu() {
	menuBirdTex.setSmooth(false);
	menuPipeTex.setSmooth(false);

	menuBirdSprite.setTexture(menuBirdTex);
	menuBirdSprite.setScale({2.0f, 2.0f});
	menuBirdSprite.setOrigin({8.0f, 8.0f});

	menuPipeSprite.setTexture(menuPipeTex);
	menuPipeSprite.setScale({4.0f, 10.0f});
	menuPipeSprite.setOrigin({8.0f, static_cast<float>(menuPipeSprite.getLocalBounds().size.y)});
}

void keyPressedMenu(GameState& game, const sf::Event::KeyPressed* event) {
	if (event->scancode == sf::Keyboard::Scancode::Space) {
		resetGame(game);
		playGame(game);
	}
	// B cycles through the background themes, wrapping back to the first one
	else if (event->scancode == sf::Keyboard::Scancode::B) {
		game.bgTheme = (game.bgTheme + 1) % THEME_COUNT;
	}
}

// Main menu screen.
void drawMenu(const GameState& game, sf::RenderWindow& window) {
	sf::Vector2u winSz = window.getSize();
	float t = game.clock.getElapsedTime().asSeconds();

	// Background preview - same one as gameplay, slowly scrolling so you can
	// see what the picked theme looks like before starting
	drawBackground(window, game.bgTheme, t * 60.0f, 0.0f);

	// Decorative pipes flanking the menu. Rotated 180° so the wide cap
	// faces up (the source texture has its cap at the bottom, anchored
	// there since that's how gamedraw.cpp positions gameplay pipes).
	// Height is capped at half the window so there's clear open sky
	// above them -- handy since the first real gameplay pipe can start
	// roughly where these leave off once we transition out of the menu.
	float groundTopY = VIEW_HEIGHT - 21.0f; // top of the ground strip drawn by background.cpp
	float pipeVisibleHeight = VIEW_HEIGHT * 0.5f;
	float pipeTexHeight = static_cast<float>(menuPipeTex.getSize().y);
	float pipeScaleY = pipeVisibleHeight / pipeTexHeight;

	menuPipeSprite.setScale({4.0f, pipeScaleY});
	menuPipeSprite.setRotation(sf::degrees(180));

	menuPipeSprite.setPosition({VIEW_WIDTH * 0.15f, groundTopY - pipeVisibleHeight});
	window.draw(menuPipeSprite);

	menuPipeSprite.setPosition({VIEW_WIDTH * 0.85f, groundTopY - pipeVisibleHeight});
	window.draw(menuPipeSprite);

	// Bobbing bird, front and center
	float bob = std::sin(t * 2.5f) * 10.0f;
	menuBirdSprite.setRotation(sf::degrees(0));
	menuBirdSprite.setPosition({VIEW_WIDTH / 2.0f, VIEW_HEIGHT * 0.38f + bob});
	window.draw(menuBirdSprite);

	// Title, centered where the placeholder box used to be
	sf::Text title(menuFont, "Flappy Bird", static_cast<unsigned int>(VIEW_WIDTH * 0.085f));
	title.setFillColor(sf::Color::White);
	title.setOutlineColor(sf::Color::Black);
	title.setOutlineThickness(3.f);
	sf::FloatRect titleBounds = title.getLocalBounds();
	title.setOrigin(titleBounds.position + titleBounds.size / 2.0f);
	title.setPosition({VIEW_WIDTH / 2.0f, VIEW_HEIGHT * 0.15f});
	window.draw(title);

	// Pulsing "Press Space to Start" prompt
	float pulse = (std::sin(t * 4.0f) + 1.0f) / 2.0f;
	sf::Text prompt(menuFont, "Press Space to Start", static_cast<unsigned int>(VIEW_HEIGHT * 0.032f));
	prompt.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(120 + pulse * 135)));
	prompt.setOutlineColor(sf::Color::Black);
	prompt.setOutlineThickness(2.f);
	sf::FloatRect promptBounds = prompt.getLocalBounds();
	prompt.setOrigin(promptBounds.position + promptBounds.size / 2.0f);
	prompt.setPosition({VIEW_WIDTH / 2.0f, VIEW_HEIGHT * 0.68f});
	window.draw(prompt);

	// Background option, shows which theme is picked
	sf::Text option(menuFont, "[B] Background: " + themes[game.bgTheme].name, static_cast<unsigned int>(VIEW_HEIGHT * 0.03f));
	option.setFillColor(sf::Color::White);
	option.setOutlineColor(sf::Color::Black);
	option.setOutlineThickness(2.f);
	sf::FloatRect optionBounds = option.getLocalBounds();
	option.setOrigin(optionBounds.position + optionBounds.size / 2.0f);
	option.setPosition({VIEW_WIDTH / 2.0f, VIEW_HEIGHT * 0.78f});
	window.draw(option);
}

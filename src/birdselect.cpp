#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include "birdselect.h"
#include "menu.h"
#include "gamedraw.h"
#include "gameconfig.h"

// The first one is the original bird. To swap any of them for different art,
// just replace the png (keep it 16x16) - nothing else needs to change.
   static const std::array<std::string, BIRD_SKIN_COUNT> birdFiles = {
   	"rsc/textures/birds/flappy.png",
   	"rsc/textures/birds/mario.png",
   	"rsc/textures/birds/steve.png",
   	"rsc/textures/birds/knight.png",
   	"rsc/textures/birds/chicken.png",
   	"rsc/textures/birds/r2d2.png",
   	"rsc/textures/birds/amongus.png",
   	"rsc/textures/bird.png"
   };

GameStatus GameStatuses::birdSelect = {
	.keyPressed = keyPressedBirdSelect,
	.mouseButtonPressed = mouseBirdSelect,
	.draw = drawBirdSelect
};

// Layout (in view coordinates, same as the rest of the menus)

// Button on the main menu: bottom right corner, just above the ground strip
static const sf::FloatRect buttonBounds({ VIEW_WIDTH - 64.0f, VIEW_HEIGHT - 21.0f - 8.0f - 56.0f }, { 56.0f, 56.0f });

// Picker screen: 4 boxes across, 2 rows
constexpr int COLUMNS = 4;
constexpr float BOX_SIZE = 96.0f;
constexpr float BOX_GAP = 24.0f;
constexpr float GRID_X = (VIEW_WIDTH - (COLUMNS * BOX_SIZE + (COLUMNS - 1) * BOX_GAP)) / 2.0f;
constexpr float GRID_Y = 90.0f;

static sf::FloatRect boxBounds(int i) {
	int col = i % COLUMNS;
	int row = i / COLUMNS;
	return sf::FloatRect({ GRID_X + col * (BOX_SIZE + BOX_GAP), GRID_Y + row * (BOX_SIZE + BOX_GAP) }, { BOX_SIZE, BOX_SIZE });
}

static sf::FloatRect doneBounds() {
	return sf::FloatRect({ VIEW_WIDTH / 2.0f - 80.0f, 326.0f }, { 160.0f, 40.0f });
}

// Centered white text with a black outline, same look as the other menu text
static void drawLabel(sf::RenderWindow& window, const std::string& str, unsigned int size, sf::Vector2f center) {
	sf::Text text(gameFont, str, size);
	text.setFillColor(sf::Color::White);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(2.0f);
	sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin(bounds.position + bounds.size / 2.0f);
	text.setPosition(center);
	window.draw(text);
}

const sf::Texture& birdSkinTexture(int skin) {
	static std::vector<sf::Texture> textures;
	if (textures.empty()) {
		textures.reserve(BIRD_SKIN_COUNT); // so the textures never move around once loaded
		for (const std::string& file : birdFiles) {
			textures.emplace_back(file); // throws if the file is missing, same as the other textures
			textures.back().setSmooth(false);
		}
	}
	return textures[std::clamp(skin, 0, BIRD_SKIN_COUNT - 1)];
}

// Menu button

void drawBirdButton(const GameState& game, sf::RenderWindow& window) {
	sf::RectangleShape box(buttonBounds.size);
	box.setPosition(buttonBounds.position);
	box.setFillColor(sf::Color(0, 0, 0, 110));
	box.setOutlineColor(sf::Color::White);
	box.setOutlineThickness(2.0f);
	window.draw(box);

	// shows the bird that's currently picked
	sf::Sprite bird(birdSkinTexture(game.birdSkin));
	bird.setOrigin({ 8.0f, 8.0f });
	bird.setScale({ 3.0f, 3.0f });
	bird.setPosition(buttonBounds.position + buttonBounds.size / 2.0f);
	window.draw(bird);

	drawLabel(window, "Bird", 10, { buttonBounds.position.x + buttonBounds.size.x / 2.0f, buttonBounds.position.y - 8.0f });
}

void mouseBirdButton(GameState& game, const sf::Event::MouseButtonPressed* event) {
	if (event->button == sf::Mouse::Button::Left) {
		sf::Vector2f mousePos = game.window->mapPixelToCoords(event->position);
		if (buttonBounds.contains(mousePos))
			game.status = GameStatuses::birdSelect;
	}
}

// Picker screen

void keyPressedBirdSelect(GameState& game, const sf::Event::KeyPressed* event) {
	// Esc or Enter goes back to the main menu
	if (event->scancode == sf::Keyboard::Scancode::Escape || event->scancode == sf::Keyboard::Scancode::Enter)
		game.status = GameStatuses::menu;
}

void mouseBirdSelect(GameState& game, const sf::Event::MouseButtonPressed* event) {
	if (event->button != sf::Mouse::Button::Left)
		return;

	sf::Vector2f mousePos = game.window->mapPixelToCoords(event->position);

	if (doneBounds().contains(mousePos)) {
		game.status = GameStatuses::menu;
		return;
	}

	for (int i = 0; i < BIRD_SKIN_COUNT; i++) {
		if (boxBounds(i).contains(mousePos)) {
			game.birdSkin = i; // stays picked, same as the background and pipes
			return;
		}
	}
}

void drawBirdSelect(const GameState& game, sf::RenderWindow& window) {
	// Main menu stays visible behind, just darkened
	drawMenu(game, window);

	sf::RectangleShape overlay(sf::Vector2f(VIEW_WIDTH, VIEW_HEIGHT));
	overlay.setFillColor(sf::Color(0, 0, 0, 150));
	window.draw(overlay);

	// Darker panel behind the picker so the menu title doesn't show through the text
	sf::RectangleShape panel(sf::Vector2f(528.0f, 368.0f));
	panel.setPosition({ (VIEW_WIDTH - 528.0f) / 2.0f, 16.0f });
	panel.setFillColor(sf::Color(0, 0, 0, 150));
	panel.setOutlineColor(sf::Color(255, 255, 255, 90));
	panel.setOutlineThickness(2.0f);
	window.draw(panel);

	drawLabel(window, "Choose Your Bird", 24, { VIEW_WIDTH / 2.0f, 48.0f });

	for (int i = 0; i < BIRD_SKIN_COUNT; i++) {
		sf::FloatRect bounds = boxBounds(i);
		bool selected = (i == game.birdSkin);

		// the picked one gets a thick gold outline
		sf::RectangleShape box(bounds.size);
		box.setPosition(bounds.position);
		box.setFillColor(sf::Color(255, 255, 255, selected ? 70 : 30));
		box.setOutlineColor(selected ? sf::Color(255, 215, 0) : sf::Color(200, 200, 200, 160));
		box.setOutlineThickness(selected ? 4.0f : 2.0f);
		window.draw(box);

		sf::Sprite bird(birdSkinTexture(i));
		bird.setOrigin({ 8.0f, 8.0f });
		bird.setScale({ 4.0f, 4.0f });
		bird.setPosition(bounds.position + bounds.size / 2.0f);
		window.draw(bird);
	}

	// Done button, same style as the buttons on the game over screen
	sf::FloatRect done = doneBounds();
	sf::RectangleShape doneBtn(done.size);
	doneBtn.setPosition(done.position);
	doneBtn.setFillColor(sf::Color(50, 150, 50));
	doneBtn.setOutlineColor(sf::Color::White);
	doneBtn.setOutlineThickness(2.0f);
	window.draw(doneBtn);

	drawLabel(window, "Done", 16, done.position + done.size / 2.0f);
}
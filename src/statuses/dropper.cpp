#include "gameconfig.h"
#include "gamedraw.h"
#include "screencalc.h"
#include "background.h"
#include "pipes.h"
#include "gamestatuses.h"
#include "statuses/birdselect.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Tuning numbers. Change these to make the mode easier or harder.
// ---------------------------------------------------------------------------
constexpr float DROP_BIRD_Y         = 100.0f;  // the bird stays at this height; the barriers move instead
constexpr float DROP_STEER_SPEED    = 260.0f;  // how fast the bird moves left/right
constexpr float DROP_MIN_SPEED      = 130.0f;  // how fast barriers rise at the start
constexpr float DROP_MAX_SPEED      = 260.0f;  // barriers never rise faster than this
constexpr float DROP_SPEED_PER_PT   = 5.0f;    // extra barrier speed gained per point
constexpr float DROP_FAST_MULT     = 1.8f;    // barrier speed multiplier while holding S / Down (soft drop)
constexpr float DROP_SPACING        = 170.0f;  // vertical distance between barriers
constexpr float DROP_THICKNESS      = 28.0f;   // how tall each barrier is
constexpr float DROP_GAP_WIDTH      = 140.0f;  // width of the hole to fall through
constexpr float DROP_MAX_GAP_SHIFT  = 250.0f;  // the next hole is never further than this from the last one
constexpr float DROP_INVULN_SECONDS = 1.0f;    // grace time after losing a life

// One barrier: a row of "pipe" across the whole screen with a hole in it.
struct Barrier {
	float y = 0.0f;     // centre height, in the same centred y-up space as GameState::bird.pos
	float gapX = 0.0f;  // centre of the hole
	bool passed = false;
};

// Mode state lives here (same approach as Flap Hunt) so GameState is unchanged.
static std::vector<Barrier> barriers;
static float distSinceSpawn = 0.0f;
static float lastGapX = 0.0f;
static float tilt = 0.0f; // -1..1, which way the bird is currently steering (just for the sprite)

static void enterDropper(GameState& game);
static void tickDropper(GameState& game, sf::RenderWindow& window);
static void keyPressedDropper(GameState& game, const sf::Event::KeyPressed* event);
static void drawDropper(const GameState& game, sf::RenderWindow& window);

GameStatus GameStatuses::dropper = {
	.keyPressed = keyPressedDropper,
	.mouseButtonPressed = nullptr,
	.draw = drawDropper,
	.tick = tickDropper,
	.enter = enterDropper
};

// Runs when the game switches to Dropper (set up by setGameStatus).
static void enterDropper(GameState& game) {
	barriers.clear();
	distSinceSpawn = DROP_SPACING; // so the first barrier spawns straight away
	lastGapX = 0.0f;
	tilt = 0.0f;

	game.score = 0;
	game.health = 3;
	game.alive = true;
	game.lastHitTime = sf::seconds(-1.0f);
	game.bird.pos = { 0.0f, DROP_BIRD_Y };
	game.bird.yVel = 0.0f;

	game.menuMusic.stop();                         // the mode menu button doesn't go through playGame()
	game.lastTick = game.clock.getElapsedTime();   // so do this here: no big dt jump on the first tick
}

static void keyPressedDropper(GameState& game, const sf::Event::KeyPressed* event) {
	if (event->scancode == sf::Keyboard::Scancode::Escape) {
		resetGame(game); // back to the menu
	}
}

// Takes a life and (if that was the last one) ends the game.
static void loseLife(GameState& game, sf::Time now) {
	if (game.health > 0) game.health--;
	game.lastHitTime = now;
	game.hitSound.play();

	if (game.health == 0) { // health is unsigned, so check for exactly 0
		game.alive = false;
		setGameStatus(game, GameStatuses::gameOver);
		game.gameOverMusic.play();
	}
}

static void tickDropper(GameState& game, sf::RenderWindow& window) {
	sf::Time now = game.clock.getElapsedTime();
	float dt = std::min((now - game.lastTick).asSeconds(), 0.05f); // cap so a lag spike can't teleport things
	game.lastTick = now;

	float speed = std::min(DROP_MIN_SPEED + game.score * DROP_SPEED_PER_PT, DROP_MAX_SPEED);

	// Steering: hold A/D or Left/Right. Hold S/Down to drop faster.
	float dir = 0.0f;
	if (window.hasFocus()) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down))
			speed *= DROP_FAST_MULT;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left))
			dir -= 1.0f;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right))
			dir += 1.0f;
	}
	game.bird.pos.x += dir * DROP_STEER_SPEED * dt;
	game.bird.pos.y = DROP_BIRD_Y;
	tilt += (dir - tilt) * std::min(1.0f, 10.0f * dt); // ease toward the steering direction

	// Move barriers up and score the ones the bird has fallen past.
	for (auto& b : barriers) {
		b.y += speed * dt;
		if (!b.passed && b.y - DROP_THICKNESS / 2.0f > game.bird.pos.y + BIRD_RADIUS) {
			b.passed = true;
			game.score++;
			game.coinSound.play();
		}
	}

	// Remove barriers that have left the top of the screen.
	std::erase_if(barriers, [](const Barrier& b) {
		return b.y - DROP_THICKNESS / 2.0f > VIEW_HEIGHT / 2.0f;
	});

	// Spawn a new barrier at the bottom every DROP_SPACING of travel.
	distSinceSpawn += speed * dt;
	if (distSinceSpawn >= DROP_SPACING) {
		distSinceSpawn = 0.0f;

		// the new hole stays within reach of the last one so every barrier can be passed
		float limit = VIEW_WIDTH / 2.0f - DROP_GAP_WIDTH / 2.0f - 30.0f;
		float shift = static_cast<float>(rand() % static_cast<int>(2 * DROP_MAX_GAP_SHIFT + 1)) - DROP_MAX_GAP_SHIFT;

		Barrier b;
		b.y = -VIEW_HEIGHT / 2.0f - DROP_THICKNESS / 2.0f;
		b.gapX = std::clamp(lastGapX + shift, -limit, limit);
		lastGapX = b.gapX;
		barriers.push_back(b);
	}

	bool invulnerable = (now - game.lastHitTime).asSeconds() <= DROP_INVULN_SECONDS;

	// Left or right edge of the screen = lose a life, and come back in the middle.
	if (game.bird.pos.x < -VIEW_WIDTH / 2.0f - BIRD_RADIUS || game.bird.pos.x > VIEW_WIDTH / 2.0f + BIRD_RADIUS) {
		game.bird.pos.x = 0.0f;
		tilt = 0.0f;
		loseLife(game, now);
		return;
	}

	// Barrier hit = lose a life (not while the grace period is running).
	if (!invulnerable) {
		for (const auto& b : barriers) {
			bool sameHeight = std::abs(b.y - game.bird.pos.y) < DROP_THICKNESS / 2.0f + BIRD_RADIUS;
			if (!sameHeight) continue;

			float gapLeft = b.gapX - DROP_GAP_WIDTH / 2.0f;
			float gapRight = b.gapX + DROP_GAP_WIDTH / 2.0f;
			bool insideGap = game.bird.pos.x - BIRD_RADIUS >= gapLeft && game.bird.pos.x + BIRD_RADIUS <= gapRight;
			if (!insideGap) {
				loseLife(game, now);
				return;
			}
		}
	}
}

// Draws one half of a barrier. "mouth" is the end next to the hole (screen coords),
// the pipe body runs away from the hole toward the screen edge.
static void drawBarrierSide(sf::RenderWindow& window, int skin, sf::Vector2f mouth, bool leftSide, float length) {
	if (length <= 0.0f) return;

	const sf::Texture& tex = pipeSkins[skin].tex;
	sf::Vector2f size(tex.getSize());

	sf::Sprite s(tex);
	s.setOrigin({ size.x / 2.0f, size.y });                  // bottom-centre of the pipe image = its mouth
	s.setScale({ DROP_THICKNESS / size.x, length / size.y }); // stretch to the barrier size
	s.setRotation(sf::degrees(leftSide ? -90.0f : 90.0f));    // lay it on its side
	s.setPosition(mouth);
	window.draw(s);
}

static void drawDropper(const GameState& game, sf::RenderWindow& window) {
	drawBackground(window, game.bgTheme, 0.0f, 0.0f);

	// Barriers
	for (const auto& b : barriers) {
		float gapLeft = b.gapX - DROP_GAP_WIDTH / 2.0f;
		float gapRight = b.gapX + DROP_GAP_WIDTH / 2.0f;

		drawBarrierSide(window, game.pipeSkin, normToScreen({ gapLeft, b.y }, window), true, gapLeft + VIEW_WIDTH / 2.0f);
		drawBarrierSide(window, game.pipeSkin, normToScreen({ gapRight, b.y }, window), false, VIEW_WIDTH / 2.0f - gapRight);
	}

	// Bird: sprite faces right, so turn it 90 degrees to point head-first downward,
	// and lean it a little the way it's steering. Blinks after a hit like Classic.
	float timeSinceHit = (game.clock.getElapsedTime() - game.lastHitTime).asSeconds();
	if (!game.alive || timeSinceHit >= DROP_INVULN_SECONDS || static_cast<int>(timeSinceHit * 10.0f) % 2 == 0) {
		birdSprite.setTexture(birdSkinTexture(game.birdSkin));
		birdSprite.setPosition(normToScreen(game.bird.pos, window));
		birdSprite.setRotation(sf::degrees(90.0f - tilt * 20.0f));
		window.draw(birdSprite);
	}
	birdSprite.setRotation(sf::degrees(0.0f));

	// HUD
	std::string uiString = "Score: " + std::to_string(game.score) + "\nLives: " + std::to_string(game.health);
	sf::Text uiText(gameFont, uiString, 16);
	uiText.setFillColor(sf::Color::White);
	uiText.setOutlineColor(sf::Color::Black);
	uiText.setOutlineThickness(2.0f);
	sf::FloatRect textBounds = uiText.getLocalBounds();
	uiText.setPosition({ VIEW_WIDTH - textBounds.size.x - 20.0f, 20.0f });
	window.draw(uiText);

	sf::Text help(gameFont, "A/D or arrows: steer   S/Down: drop faster   [ESC] Menu", 10);
	help.setFillColor(sf::Color::White);
	help.setOutlineColor(sf::Color::Black);
	help.setOutlineThickness(2.0f);
	help.setPosition({ 20.0f, 20.0f });
	window.draw(help);
}
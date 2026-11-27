#include "gamestatuses.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "gameconfig.h"
#include "gamedraw.h"
#include "screencalc.h"
#include "background.h"
#include "gamestatuses.h"
#include "statuses/birdselect.h"

// Flappy Darts: one fixed screen, normal Flappy controls, THREE throws.
// Press SPACE to launch the bird from the left. It flies right on its own and
// you flap (SPACE) to steer it into the targets. Smaller and moving targets
// are worth more (size sets the points, moving doubles them). Every throw gets
// a brand-new set of targets. A throw ends when you hit a target, hit the
// ground, or fly off the right side. After 3 throws the round ends and the
// normal Game Over screen shows (the HUD keeps your total score visible).

// ---------------------------------------------------------------------------
// Tuning numbers. Change these to make the mode easier or harder.
// ---------------------------------------------------------------------------
constexpr int   DARTS_THROWS         = 3;      // throws per round
constexpr float DARTS_FLY_SPEED      = 170.0f; // how fast the bird flies to the right
constexpr float DARTS_START_X        = -VIEW_WIDTH / 2.0f + 50.0f;
constexpr float DARTS_GROUND         = 21.0f;  // height of the ground strip (same value Classic uses)
constexpr float DARTS_RESULT_SECONDS = 1.3f;   // how long "+50" / "MISS" stays up between throws

// ---------------------------------------------------------------------------
// Targets
// ---------------------------------------------------------------------------
struct Target {
	float x = 0.0f;
	float baseY = 0.0f;   // centre height (for moving targets: the middle of their path)
	float radius = 20.0f;
	int points = 0;
	float amplitude = 0.0f; // 0 = doesn't move; otherwise how far it bobs up/down
	float speed = 0.0f;
	float phase = 0.0f;
	bool hit = false;       // the target the bird just hit (shown greyed while the result is up)
};

// Scoring rule: points come from the target's size, doubled if it moves.
//   size:    Big 10   Medium 25   Small 50
//   moving:  x2   (Big 20, Medium 50, Small 100)
// Each throw deals five targets with a random size and a random moving setting.
struct TargetSize { float radius; int points; };

constexpr TargetSize TARGET_SIZES[3] = {
	{ 30.0f, 10 }, // big, easy
	{ 20.0f, 25 }, // medium
	{ 13.0f, 50 }  // small
};
constexpr int   MOVING_CHANCE_PERCENT = 40;   // chance that a target moves
constexpr int   MOVING_MULTIPLIER     = 2;    // moving targets are worth this many times more
constexpr float MOVING_AMPLITUDE      = 60.0f;// how far a moving target bobs up and down
constexpr float MOVING_SPEED          = 1.8f; // how fast it bobs
constexpr float TARGET_COLUMNS[5] = { -110.0f, 10.0f, 130.0f, 250.0f, 340.0f };

enum class Phase { Ready, Flying, Result, Done }; // Done = round finished, Game Over screen is up

// Mode state lives here (same approach as Flap Hunt) so GameState is unchanged.
static std::vector<Target> targets;
static Phase phase = Phase::Ready;
static int throwNumber = 0;          // 0-based index of the current throw
static int throwPoints = 0;          // what the current/last throw scored
static sf::Time resultStart;
static sf::Vector2f resultPos;       // where the "+N" popup floats
static std::string resultText;

static void enterDarts(GameState& game);
static void tickDarts(GameState& game, sf::RenderWindow& window);
static void keyPressedDarts(GameState& game, const sf::Event::KeyPressed* event);
static void drawDarts(const GameState& game, sf::RenderWindow& window);

GameStatus GameStatuses::darts = {
	.keyPressed = keyPressedDarts,
	.mouseButtonPressed = nullptr,
	.draw = drawDarts,
	.tick = tickDarts,
	.enter = enterDarts
};

// Where a target is right now (moving ones bob up and down).
static float targetY(const Target& t, float seconds) {
	if (t.amplitude == 0.0f) return t.baseY;
	return t.baseY + t.amplitude * std::sin(t.phase + seconds * t.speed);
}

// Deals a fresh set of five targets (random size, random moving or still).
static void spawnTargets() {
	targets.clear();
	for (int col = 0; col < 5; col++) {
		const TargetSize& size = TARGET_SIZES[std::rand() % 3];
		bool moving = (std::rand() % 100) < MOVING_CHANCE_PERCENT;

		Target t;
		t.x = TARGET_COLUMNS[col];
		t.radius = size.radius;
		t.points = size.points * (moving ? MOVING_MULTIPLIER : 1);
		t.amplitude = moving ? MOVING_AMPLITUDE : 0.0f;
		t.speed = moving ? MOVING_SPEED : 0.0f;
		t.phase = static_cast<float>(std::rand() % 628) / 100.0f; // random start point in the bob
		// moving targets bob around the middle; still ones can sit anywhere
		t.baseY = moving
			? static_cast<float>(std::rand() % 81) - 40.0f
			: static_cast<float>(std::rand() % 201) - 100.0f;
		targets.push_back(t);
	}
}

static void readyNextThrow(GameState& game) {
	phase = Phase::Ready;
	throwPoints = 0;
	game.bird.pos = { DARTS_START_X, 0.0f };
	game.bird.yVel = 0.0f;
	spawnTargets(); // every throw gets a new set of targets
}

// Runs when the game switches to Darts (set up by setGameStatus).
static void enterDarts(GameState& game) {
	game.score = 0;
	game.health = 3;
	game.alive = true;
	game.lastHitTime = sf::seconds(-1.0f);
	game.menuMusic.stop();                       // the mode menu button doesn't go through playGame()
	game.lastTick = game.clock.getElapsedTime(); // no big dt jump on the first tick

	throwNumber = 0;
	resultText.clear();
	readyNextThrow(game); // also deals the first set of targets
}

static void finishThrow(GameState& game, sf::Time now, int points, sf::Vector2f popupPos) {
	game.score += points;
	throwPoints = points;
	resultText = points > 0 ? "+" + std::to_string(points) : "MISS";
	resultPos = popupPos;
	resultStart = now;
	phase = Phase::Result;

	if (points > 0) game.coinSound.play();
	else game.hitSound.play();
}

static void keyPressedDarts(GameState& game, const sf::Event::KeyPressed* event) {
	if (event->scancode == sf::Keyboard::Scancode::Escape) {
		resetGame(game); // back to the menu
		return;
	}

	if (event->scancode != sf::Keyboard::Scancode::Space)
		return;

	if (phase == Phase::Ready) {
		phase = Phase::Flying;
		game.bird.yVel = BIRD_JUMP_VEL; // the launch is the first flap
		game.flapSound.play();
	}
	else if (phase == Phase::Flying) {
		game.bird.yVel = BIRD_JUMP_VEL;
		game.flapSound.play();
	}
}

static void tickDarts(GameState& game, sf::RenderWindow& window) {
	sf::Time now = game.clock.getElapsedTime();
	float dt = std::min((now - game.lastTick).asSeconds(), 0.05f); // cap so a lag spike can't teleport the bird
	game.lastTick = now;
	float seconds = now.asSeconds();

	if (phase == Phase::Ready) {
		// the bird bobs gently while you get ready
		game.bird.pos.y = 8.0f * std::sin(seconds * 3.0f);
		return;
	}

	if (phase == Phase::Result) {
		if ((now - resultStart).asSeconds() >= DARTS_RESULT_SECONDS) {
			throwNumber++;
			if (throwNumber >= DARTS_THROWS) {
				phase = Phase::Done;
				game.alive = false;
				setGameStatus(game, GameStatuses::gameOver); // the shared Game Over screen (Play Again, Main Menu)
				game.gameOverMusic.play();
			}
			else {
				readyNextThrow(game);
			}
		}
		return;
	}

	if (phase != Phase::Flying)
		return;

	// Normal Flappy physics, plus a steady push to the right.
	game.bird.pos.x += DARTS_FLY_SPEED * dt;
	game.bird.pos.y += game.bird.yVel * dt;
	game.bird.yVel -= GRAVITY_CONST * dt;

	// Ceiling: just stop at the top of the screen.
	if (game.bird.pos.y + BIRD_RADIUS > VIEW_HEIGHT / 2.0f) {
		game.bird.pos.y = VIEW_HEIGHT / 2.0f - BIRD_RADIUS;
		game.bird.yVel = 0.0f;
	}

	// Hit a target? It scores, gets knocked out, and the throw ends.
	for (auto& t : targets) {
		if (t.hit) continue;
		sf::Vector2f tp(t.x, targetY(t, seconds));
		float dx = game.bird.pos.x - tp.x;
		float dy = game.bird.pos.y - tp.y;
		float reach = BIRD_RADIUS + t.radius;
		if (dx * dx + dy * dy <= reach * reach) {
			t.hit = true;
			finishThrow(game, now, t.points, { tp.x, tp.y + t.radius + 14.0f });
			return;
		}
	}

	// Ground or off the right side = a miss.
	if (game.bird.pos.y - BIRD_RADIUS <= -VIEW_HEIGHT / 2.0f + DARTS_GROUND) {
		game.bird.pos.y = -VIEW_HEIGHT / 2.0f + DARTS_GROUND + BIRD_RADIUS;
		finishThrow(game, now, 0, { game.bird.pos.x, game.bird.pos.y + 30.0f });
	}
	else if (game.bird.pos.x > VIEW_WIDTH / 2.0f + BIRD_RADIUS) {
		finishThrow(game, now, 0, { VIEW_WIDTH / 2.0f - 70.0f, game.bird.pos.y });
	}
}

// Draws text centred on a point (centred y-up coordinates).
static void drawCenteredText(sf::RenderWindow& window, const std::string& str, unsigned int size,
                             sf::Vector2f pos, sf::Color fill, float outline = 2.0f) {
	sf::Text text(gameFont, str, size);
	text.setFillColor(fill);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(outline);
	sf::FloatRect b = text.getLocalBounds();
	text.setOrigin(b.position + b.size / 2.0f);
	text.setPosition(pos);
	window.draw(text);
}

static void drawTarget(sf::RenderWindow& window, const Target& t, float seconds) {
	sf::Vector2f center = normToScreen({ t.x, targetY(t, seconds) }, window);
	bool out = t.hit;
	sf::Color red = out ? sf::Color(120, 120, 120, 110) : sf::Color(210, 40, 40);
	sf::Color white = out ? sf::Color(200, 200, 200, 110) : sf::Color(245, 245, 245);

	// bullseye: red ring, white ring, red centre
	const float rings[3] = { 1.0f, 0.66f, 0.33f };
	const sf::Color colours[3] = { red, white, red };
	for (int i = 0; i < 3; i++) {
		float r = t.radius * rings[i];
		sf::CircleShape ring(r);
		ring.setOrigin({ r, r });
		ring.setPosition(center);
		ring.setFillColor(colours[i]);
		if (i == 0) {
			ring.setOutlineColor(sf::Color::Black);
			ring.setOutlineThickness(2.0f);
		}
		window.draw(ring);
	}

	if (!out)
		drawCenteredText(window, std::to_string(t.points), 10, { center.x, center.y - t.radius - 12.0f }, sf::Color::White);
}

static void drawDarts(const GameState& game, sf::RenderWindow& window) {
	float seconds = game.clock.getElapsedTime().asSeconds();

	drawBackground(window, game.bgTheme, 0.0f, 0.0f);

	for (const auto& t : targets)
		drawTarget(window, t, seconds);

	// Bird (rotated the same way Classic does it)
	birdSprite.setTexture(birdSkinTexture(game.birdSkin));
	birdSprite.setPosition(normToScreen(game.bird.pos, window));
	birdSprite.setRotation(sf::degrees(phase == Phase::Flying
		? std::clamp(-40.0f - (game.bird.yVel - BIRD_JUMP_VEL) / 10.0f, -40.0f, 60.0f)
		: 0.0f));
	window.draw(birdSprite);
	birdSprite.setRotation(sf::degrees(0.0f));

	// HUD
	int shownThrow = std::min(throwNumber + 1, DARTS_THROWS);
	std::string hud = "Throw " + std::to_string(shownThrow) + "/" + std::to_string(DARTS_THROWS) +
	                  "\nScore: " + std::to_string(game.score);
	sf::Text hudText(gameFont, hud, 16);
	hudText.setFillColor(sf::Color::White);
	hudText.setOutlineColor(sf::Color::Black);
	hudText.setOutlineThickness(2.0f);
	sf::FloatRect hb = hudText.getLocalBounds();
	hudText.setPosition({ VIEW_WIDTH - hb.size.x - 20.0f, 20.0f });
	window.draw(hudText);

	sf::Text help(gameFont, "SPACE: flap    [ESC] Menu", 10);
	help.setFillColor(sf::Color::White);
	help.setOutlineColor(sf::Color::Black);
	help.setOutlineThickness(2.0f);
	help.setPosition({ 20.0f, 20.0f });
	window.draw(help);

	if (phase == Phase::Ready) {
		// two short lines in the free space under the bird's start spot, so it never sits on a target
		drawCenteredText(window, "Press SPACE\nto throw", 12, { 130.0f, VIEW_HEIGHT * 0.74f }, sf::Color::White);
	}

	if (phase == Phase::Result) {
		sf::Vector2f p = normToScreen(resultPos, window);
		p.x = std::clamp(p.x, 60.0f, VIEW_WIDTH - 60.0f);
		drawCenteredText(window, resultText, 20, p, throwPoints > 0 ? sf::Color(255, 230, 80) : sf::Color(255, 90, 90), 3.0f);
	}
}
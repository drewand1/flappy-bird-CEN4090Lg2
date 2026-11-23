#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include "gameconfig.h"
#include "gamedraw.h"
#include "collision.h"
#include "background.h"
#include "pipes.h"
#include "gamestatuses.h"

constexpr float GROUND = 21.0f;
constexpr float FLOOR = -VIEW_HEIGHT / 2.0f + GROUND;
constexpr float TEE_X = 80.0f;
constexpr float MAX_POWER = 520.0f;
constexpr float PULL_RANGE = 120.0f;
constexpr float FLYOVER_TIME = 2.5f;
constexpr float OOPS_TIME = 1.0f;
constexpr float NEST_WIDTH = 70.0f;
constexpr float NEST_HEIGHT = 22.0f;

struct Hole {
	std::string name;
	int par;
	float nestX;
	std::vector<Pipe> pipes;
};

Hole course[] = {
	{"Warm Up", 4, 1200.0f, {
		{500.0f, 170.0f, 0.0f},
		{850.0f, 160.0f, 30.0f}
	}},
	{"Low Road", 4, 1400.0f, {
		{450.0f, 150.0f, -90.0f},
		{750.0f, 140.0f, -100.0f},
		{1050.0f, 150.0f, -80.0f}
	}},
	{"Up and Over", 8, 1400.0f, {
		{450.0f, 150.0f, 80.0f},
		{750.0f, 140.0f, 110.0f},
		{1050.0f, 150.0f, 80.0f}
	}},
	{"Zigzag", 14, 2000.0f, {
		{450.0f, 140.0f, 90.0f},
		{700.0f, 140.0f, -80.0f},
		{950.0f, 140.0f, 100.0f},
		{1200.0f, 140.0f, -100.0f},
		{1450.0f, 140.0f, 60.0f},
		{1700.0f, 140.0f, -60.0f}
	}},
	{"The Needle", 18, 2300.0f, {
		{450.0f, 120.0f, -40.0f},
		{700.0f, 120.0f, 60.0f},
		{950.0f, 120.0f, -80.0f},
		{1200.0f, 120.0f, 100.0f},
		{1450.0f, 120.0f, -100.0f},
		{1700.0f, 120.0f, 20.0f},
		{1950.0f, 120.0f, -60.0f}
	}}
};
constexpr int HOLE_COUNT = sizeof(course) / sizeof(course[0]);

enum Stage { FLYOVER, AIMING, FLYING, OOPS, SUNK, ROUND_OVER };

static Stage stage = FLYOVER;
static int hole = 0;
static int flaps = 0;
static int totalFlaps = 0;
static int totalPar = 0;
static int lastPipe = -1;
static int perch = -1;
static float xVel = 0.0f;
static float cam = 0.0f;
static float timer = 0.0f;
static std::string oops;
static bool pulling = false;
static sf::Vector2f slingStart;

float camLimit() {
	return std::max(0.0f, course[hole].nestX + 150.0f - VIEW_WIDTH);
}

sf::Vector2f onScreen(sf::Vector2f spot) {
	return {spot.x - cam, VIEW_HEIGHT / 2.0f - spot.y};
}

bool slingshot(sf::Vector2f mouse, sf::Vector2f& launch) {
	sf::Vector2f pull = slingStart - mouse;
	float dist = std::sqrt(pull.x * pull.x + pull.y * pull.y);
	if (dist < 5.0f) return false;

	float power = std::min(dist, PULL_RANGE) / PULL_RANGE * MAX_POWER;
	launch = {pull.x / dist * power, -pull.y / dist * power};
	return true;
}

void perchBird(GameState& game) {
	if (perch < 0) {
		game.bird.pos = {TEE_X, FLOOR + BIRD_RADIUS};
	}
	else {
		const Pipe& post = game.pipes[perch];
		float lip = post.gapPos - post.gapHeight / 2.0f;
		game.bird.pos = {post.xPos + PIPE_HALF_WIDTH, lip + BIRD_RADIUS + 1.0f};
	}
	game.bird.yVel = 0.0f;
	xVel = 0.0f;
	pulling = false;

	lastPipe = perch;
	for (int i = 0; i < game.pipes.size(); i++)
		game.pipes[i].passed = i <= perch;

	stage = AIMING;
}

void teeOff(GameState& game, int next) {
	hole = next;
	flaps = 0;
	perch = -1;
	game.pipes = course[hole].pipes;
	perchBird(game);

	cam = camLimit();
	timer = 0.0f;
	stage = FLYOVER;
}

//floor is +1 and back to the last pipe passed, a pipe is +1 and back two, the first pipe is +2 and back to the start
void penalty(GameState& game, int hit) {
	if (hit < 0) {
		flaps++;
		perch = lastPipe;
		if (hit == -1)
			oops = "Hit the floor! +1";
		else
			oops = "Flew past the nest! +1";
	}
	else if (hit == 0) {
		flaps += 2;
		perch = -1;
		oops = "Hit the first pipe! +2";
	}
	else {
		flaps++;
		perch = std::max(hit - 2, -1);
		oops = "Hit a pipe! +1\nBack two pipes";
	}

	game.hitSound.play();
	timer = 0.0f;
	stage = OOPS;
}

void nextHole(GameState& game) {
	if (hole + 1 < HOLE_COUNT)
		teeOff(game, hole + 1);
	else
		stage = ROUND_OVER;
}

void enterGolf(GameState& game) {
	game.menuMusic.stop();
	game.lastTick = game.clock.getElapsedTime();
	totalFlaps = 0;
	totalPar = 0;
	teeOff(game, 0);
}

void tickGolf(GameState& game, sf::RenderWindow& window) {
	sf::Time now = game.clock.getElapsedTime();
	float dt = (now - game.lastTick).asSeconds();
	game.lastTick = now;
	Bird& bird = game.bird;
	const Hole& here = course[hole];

	if (stage == FLYOVER) {
		timer += dt;
		float t = std::min(timer / FLYOVER_TIME, 1.0f);
		cam = camLimit() * (1.0f - t);
		if (t >= 1.0f) stage = AIMING;
	}
	else if (stage == OOPS) {
		timer += dt;
		if (timer >= OOPS_TIME) perchBird(game);
	}
	else if (stage == AIMING && pulling && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
		pulling = false;
		sf::Vector2f launch;
		if (slingshot(window.mapPixelToCoords(sf::Mouse::getPosition(window)), launch)) {
			xVel = launch.x;
			bird.yVel = launch.y;
			flaps++;
			game.flapSound.play();
			stage = FLYING;
		}
	}
	else if (stage == FLYING) {
		bird.pos.x += xVel * dt;
		bird.pos.y += bird.yVel * dt;
		bird.yVel -= GRAVITY_CONST * dt;

		if (bird.pos.y + BIRD_RADIUS > VIEW_HEIGHT / 2.0f) {
			bird.pos.y = VIEW_HEIGHT / 2.0f - BIRD_RADIUS;
			if (bird.yVel > 0.0f) bird.yVel = 0.0f;
		}

		for (int i = 0; i < game.pipes.size(); i++) {
			Pipe& post = game.pipes[i];
			if (!post.passed && bird.pos.x - BIRD_RADIUS > post.xPos + PIPE_HALF_WIDTH) {
				post.passed = true;
				lastPipe = i;
				game.coinSound.play();
			}
		}

		for (int i = 0; i < game.pipes.size(); i++) {
			if (checkPipeCollision(bird, game.pipes[i])) {
				penalty(game, i);
				return;
			}
		}

		if (bird.yVel < 0.0f && std::abs(bird.pos.x - here.nestX) < NEST_WIDTH / 2.0f && bird.pos.y - BIRD_RADIUS <= FLOOR + NEST_HEIGHT) {
			bird.pos = {here.nestX, FLOOR + NEST_HEIGHT};
			bird.yVel = 0.0f;
			xVel = 0.0f;
			totalFlaps += flaps;
			totalPar += here.par;
			stage = SUNK;
			return;
		}

		if (bird.pos.x > here.nestX + 400.0f) {
			penalty(game, -2);
			return;
		}

		if (bird.pos.y - BIRD_RADIUS <= FLOOR) {
			bird.pos.y = FLOOR + BIRD_RADIUS;
			penalty(game, -1);
			return;
		}
	}

	if (stage != FLYOVER) {
		float target = std::clamp(bird.pos.x - 200.0f, 0.0f, camLimit());
		cam += (target - cam) * 5.0f * dt;
	}
	game.scrollDistance = cam;
}

void keyPressedGolf(GameState& game, const sf::Event::KeyPressed* event) {
	if (event->scancode == sf::Keyboard::Scancode::Escape) {
		resetGame(game);
		return;
	}
	if (event->scancode != sf::Keyboard::Scancode::Space) return;

	if (stage == FLYOVER) {
		stage = AIMING;
	}
	else if (stage == FLYING) {
		game.bird.yVel = BIRD_JUMP_VEL;
		flaps++;
		game.flapSound.play();
	}
	else if (stage == SUNK) {
		nextHole(game);
	}
	else if (stage == ROUND_OVER) {
		resetGame(game);
	}
}

void mousePressedGolf(GameState& game, const sf::Event::MouseButtonPressed* event) {
	if (event->button != sf::Mouse::Button::Left) return;

	if (stage == FLYOVER) {
		stage = AIMING;
	}
	else if (stage == AIMING) {
		pulling = true;
		slingStart = game.window->mapPixelToCoords(event->position);
	}
	else if (stage == FLYING) {
		game.bird.yVel = BIRD_JUMP_VEL;
		flaps++;
		game.flapSound.play();
	}
	else if (stage == SUNK) {
		nextHole(game);
	}
	else if (stage == ROUND_OVER) {
		resetGame(game);
	}
}

void golfText(sf::RenderWindow& window, const std::string& words, unsigned int size, float y) {
	sf::Text label(gameFont, words, size);
	label.setFillColor(sf::Color::White);
	label.setOutlineColor(sf::Color::Black);
	label.setOutlineThickness(2.0f);
	label.setLineSpacing(1.4f);
	sf::FloatRect bounds = label.getLocalBounds();
	label.setOrigin(bounds.position + bounds.size / 2.0f);
	label.setPosition({VIEW_WIDTH / 2.0f, y});
	window.draw(label);
}

void drawGolf(const GameState& game, sf::RenderWindow& window) {
	const Hole& here = course[hole];
	drawBackground(window, game.bgTheme, cam, game.bird.pos.y);

	for (const Pipe& post : game.pipes) {
		float x = post.xPos - cam;
		if (x < -100.0f || x > VIEW_WIDTH + 100.0f) continue;
		float gapTop = post.gapPos + post.gapHeight / 2.0f;
		float gapBottom = post.gapPos - post.gapHeight / 2.0f;
		drawPipe(window, game.pipeSkin, {x, VIEW_HEIGHT / 2.0f - gapTop}, false, VIEW_HEIGHT / 2.0f - gapTop);
		drawPipe(window, game.pipeSkin, {x, VIEW_HEIGHT / 2.0f - gapBottom}, true, gapBottom - FLOOR);
	}

	sf::Vector2f nest = onScreen({here.nestX, FLOOR});
	sf::RectangleShape stick({3.0f, 70.0f});
	stick.setPosition({nest.x + NEST_WIDTH / 2.0f - 12.0f, nest.y - 70.0f});
	stick.setFillColor(sf::Color::White);
	window.draw(stick);

	sf::ConvexShape flag(3);
	flag.setPoint(0, {nest.x + NEST_WIDTH / 2.0f - 9.0f, nest.y - 70.0f});
	flag.setPoint(1, {nest.x + NEST_WIDTH / 2.0f + 22.0f, nest.y - 60.0f});
	flag.setPoint(2, {nest.x + NEST_WIDTH / 2.0f - 9.0f, nest.y - 50.0f});
	flag.setFillColor(sf::Color::Red);
	window.draw(flag);

	sf::CircleShape twigs(NEST_WIDTH / 2.0f);
	twigs.setOrigin({NEST_WIDTH / 2.0f, NEST_WIDTH / 2.0f});
	twigs.setScale({1.0f, 0.6f});
	twigs.setPosition({nest.x, nest.y - NEST_HEIGHT / 2.0f});
	twigs.setFillColor(sf::Color(139, 90, 43));
	twigs.setOutlineColor(sf::Color(90, 55, 25));
	twigs.setOutlineThickness(3.0f);
	window.draw(twigs);

	sf::CircleShape hollow(NEST_WIDTH / 2.0f - 8.0f);
	hollow.setOrigin({NEST_WIDTH / 2.0f - 8.0f, NEST_WIDTH / 2.0f - 8.0f});
	hollow.setScale({1.0f, 0.3f});
	hollow.setPosition({nest.x, nest.y - NEST_HEIGHT});
	hollow.setFillColor(sf::Color(70, 40, 20));
	window.draw(hollow);

	sf::Vector2f launch;
	if (stage == AIMING && pulling && slingshot(window.mapPixelToCoords(sf::Mouse::getPosition(window)), launch)) {
		for (int i = 1; i <= 12; i++) {
			float t = i * 0.08f;
			sf::Vector2f spot = {game.bird.pos.x + launch.x * t, game.bird.pos.y + launch.y * t - 0.5f * GRAVITY_CONST * t * t};
			sf::CircleShape dot(4.0f);
			dot.setOrigin({4.0f, 4.0f});
			dot.setPosition(onScreen(spot));
			dot.setFillColor(sf::Color(255, 255, 255, 255 - i * 15));
			dot.setOutlineColor(sf::Color(0, 0, 0, 255 - i * 15));
			dot.setOutlineThickness(1.5f);
			window.draw(dot);
		}
	}

	float tilt = 0.0f;
	if (stage == FLYING)
		tilt = std::clamp(-40 - (game.bird.yVel - BIRD_JUMP_VEL) / 10.0f, -40.0f, 60.0f);
	birdSprite.setTexture(birdSkinTexture(game.birdSkin));
	birdSprite.setPosition(onScreen(game.bird.pos));
	birdSprite.setRotation(sf::degrees(tilt));
	window.draw(birdSprite);

	sf::Text scorecard(gameFont, "FLAPPY GOLF | Hole " + std::to_string(hole + 1) + "/" + std::to_string(HOLE_COUNT) + ": " + here.name +
		"\nFlaps: " + std::to_string(flaps) + "  Par: " + std::to_string(here.par) + "\n[ESC] Menu", 14);
	scorecard.setFillColor(sf::Color::White);
	scorecard.setOutlineColor(sf::Color::Black);
	scorecard.setOutlineThickness(2.0f);
	scorecard.setLineSpacing(1.4f);
	scorecard.setPosition({20.0f, 20.0f});
	window.draw(scorecard);

	if (stage == FLYOVER) {
		golfText(window, "Hole " + std::to_string(hole + 1) + ": " + here.name + "\nPar " + std::to_string(here.par), 24, VIEW_HEIGHT * 0.4f);
	}
	else if (stage == AIMING) {
		golfText(window, "Click and drag back to aim\nLet go to launch", 14, VIEW_HEIGHT * 0.3f);
	}
	else if (stage == OOPS) {
		golfText(window, oops, 20, VIEW_HEIGHT * 0.4f);
	}
	else if (stage == SUNK || stage == ROUND_OVER) {
		sf::RectangleShape shade({VIEW_WIDTH, VIEW_HEIGHT});
		shade.setFillColor(sf::Color(0, 0, 0, 150));
		window.draw(shade);

		std::string words;
		if (stage == SUNK) {
			int over = flaps - here.par;
			std::string call = "+" + std::to_string(over);
			if (flaps == 1) call = "Hole in one!";
			else if (over <= -2) call = "Eagle!";
			else if (over == -1) call = "Birdie!";
			else if (over == 0) call = "Par";
			else if (over == 1) call = "Bogey";

			words = "In the nest!\n" + call + "\nFlaps: " + std::to_string(flaps) + "  Par: " + std::to_string(here.par);
			words += hole + 1 < HOLE_COUNT ? "\n\nSpace for the next hole" : "\n\nSpace to see your round";
		}
		else {
			words = "Round over!\nTotal flaps: " + std::to_string(totalFlaps) + "  Par: " + std::to_string(totalPar) + "\n\nSpace for the menu";
		}
		golfText(window, words, 20, VIEW_HEIGHT * 0.45f);
	}
}

GameStatus GameStatuses::golf = {
	.keyPressed = keyPressedGolf,
	.mouseButtonPressed = mousePressedGolf,
	.draw = drawGolf,
	.tick = tickGolf,
	.enter = enterGolf
};
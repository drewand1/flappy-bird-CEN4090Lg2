#include "pipes.h"

constexpr float PIPE_MIN_LENGTH = 320.0f;

PipeSkin pipeSkins[PIPE_COUNT] = {
	{"Classic", "classic.png", 2.0f, false, 80.0f, 0, 0},
	{"Mastersword", "pipe2.png", 1.0f, true, 28.0f, 8, 27},
	{"Lightsaber", "pipe3.png", 1.0f, true, 24.0f, 5, 28},
	{"Plant", "pipe4.png", 1.0f, true, 48.0f, 42, 0}
};

void initPipes() {
	for (PipeSkin& livery : pipeSkins) {
		if (!livery.tex.loadFromFile("rsc/textures/pipes/" + livery.file)) {
			livery.tex = pipeSkins[0].tex;
			livery.stretch = pipeSkins[0].stretch;
			livery.fit = pipeSkins[0].fit;
			livery.width = pipeSkins[0].width;
		}

		livery.tex.setSmooth(livery.tex.getSize().x > 64);
		livery.rect = sf::IntRect({0, 0}, sf::Vector2i(livery.tex.getSize()));
	}
}

void setPipeSkin(sf::Sprite& cutout, int skin, float width) {
	const PipeSkin& livery = pipeSkins[skin];
	sf::Vector2f sheet(livery.rect.size);
	float scaleX = width / sheet.x;
	float scaleY = scaleX * livery.stretch;

	if (sheet.y * scaleY < PIPE_MIN_LENGTH)
		scaleY = PIPE_MIN_LENGTH / sheet.y;

	cutout.setTexture(livery.tex);
	cutout.setTextureRect(livery.rect);
	cutout.setScale({scaleX, scaleY});
	cutout.setOrigin({sheet.x / 2.0f, sheet.y});
}

void drawSlice(sf::RenderWindow& window, const sf::RenderStates& pose, const PipeSkin& livery, int firstRow, int rowCount, float y, float scaleY) {
	sf::Sprite cutout(livery.tex, sf::IntRect({0, firstRow}, {livery.rect.size.x, rowCount}));
	cutout.setScale({livery.width / livery.rect.size.x, scaleY});
	cutout.setPosition({-livery.width / 2.0f, y - rowCount * scaleY});
	window.draw(cutout, pose);
}

void drawPipe(sf::RenderWindow& window, int skin, sf::Vector2f mouth, bool flip, float length) {
	const PipeSkin& livery = pipeSkins[skin];

	if (!livery.fit) {
		sf::Sprite cutout(livery.tex);
		setPipeSkin(cutout, skin, livery.width);
		cutout.setRotation(sf::degrees(flip ? 180 : 0));
		cutout.setPosition(mouth);
		window.draw(cutout);
		return;
	}

	sf::Transform pivot;
	pivot.translate(mouth);
	if (flip) pivot.rotate(sf::degrees(180));
	sf::RenderStates pose(pivot);

	float scale = livery.width / livery.rect.size.x;
	int rows = livery.rect.size.y;
	int midRows = rows - livery.tip - livery.hilt;
	float tipLen = livery.tip * scale;
	float hiltLen = livery.hilt * scale;
	float midLen = length - tipLen - hiltLen;

	if (midLen < 0) midLen = 0;

	drawSlice(window, pose, livery, rows - livery.tip, livery.tip, 0.0f, scale);
	drawSlice(window, pose, livery, livery.hilt, midRows, -tipLen, midLen / midRows);
	drawSlice(window, pose, livery, 0, livery.hilt, -tipLen - midLen, scale);
}
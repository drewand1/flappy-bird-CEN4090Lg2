#include <cmath>
#include "background.h"
#include "gameconfig.h"

constexpr float SWAY = 0.1f;  // 0 turns the vertical parallax off
constexpr float SKY_DEPTH = 2.0f;  // how fast the sky scrolls, 1 = pipe speed, 2 = half...
struct Layer {
	float depth; // 1 = pipe speed, 2 = half, 4 = quarter...
	float y;     // offset up from the bottom of the window
};

// back to front: back, middle, front, ground. every theme uses the same spots
// ground's offset is negative so most of it sits below the screen
Layer layers[4] = {
	{8.0f, 66.0f},
	{4.0f, 34.0f},
	{2.0f, 12.0f},
	{1.0f, -27.0f}
};

const std::string files[4] = {"back.png", "middle.png", "front.png", "ground.png"};

// to add a theme make a folder in rsc/textures/backgrounds/ and change a line here
Theme themes[THEME_COUNT] = {
	{"Classic", "classic", 3.0f},
	{"Theme 2", "theme2", 3.0f},
	{"Theme 3", "theme3", 3.0f},
	{"Theme 4", "theme4", 3.0f}
};

void initBackground() {
	for (Theme& t : themes) {
		std::string dir = "rsc/textures/backgrounds/" + t.folder + "/";

		// sky is the only one you need, uses the classic sky if its missing
		if (!t.sky.loadFromFile(dir + "sky.png"))
			t.sky = themes[0].sky;

		// big pictures look better smoothed, the tiny pixel art classic sky stays sharp
		t.sky.setSmooth(t.sky.getSize().x > 64);

		// the rest are optional, missing ones just dont get drawn
		for (int i = 0; i < 4; i++) {
			if (!t.tex[i].loadFromFile(dir + files[i]))
				continue;

			t.has[i] = true;
			t.tex[i].setSmooth(false);
			t.tex[i].setRepeated(true); // wraps by sliding the texture rect

			// fill under the layer with whatever color the bottom of the png is
			sf::Image img = t.tex[i].copyToImage();
			t.under[i] = img.getPixel({img.getSize().x / 2, img.getSize().y - 1});
		}
	}
}

void drawBackground(sf::RenderWindow& window, int theme, float scrollDistance, float cameraHeight) {
	const Theme& t = themes[theme];
	sf::Vector2f win(VIEW_WIDTH, VIEW_HEIGHT);
	sf::Vector2u skySz = t.sky.getSize();

	// sky is stretched to the screen and scrolls slowly. every other copy is flipped
	// so the edges always line up, otherwise you'd see a seam where it repeats
	float skyScroll = std::fmod(scrollDistance / SKY_DEPTH, win.x * 2.0f);
	for (int k = 0; k < 3; k++) {
		float x = k * win.x - skyScroll;
		float scaleX = win.x / skySz.x;

		// odd ones get flipped. flipped sprites draw to the left of their position so shift it over
		if (k % 2 == 1) {
			scaleX = -scaleX;
			x += win.x;
		}

		sf::Sprite sky(t.sky);
		sky.setScale({scaleX, win.y / skySz.y});
		sky.setPosition({x, 0.0f});
		window.draw(sky);
	}

	int w = static_cast<int>(std::ceil(win.x / t.scale)) + 1; // +1 or the right edge shows a gap

	for (int i = 0; i < 4; i++) {
		if (!t.has[i]) continue;

		const Layer& l = layers[i];
		sf::Vector2u ts = t.tex[i].getSize();
		float scroll = scrollDistance / l.depth;

		// rect only moves in whole texels (3 screen px at scale 3) which looks choppy,
		// so nudge the sprite by the leftover
		int px = static_cast<int>(scroll / t.scale);
		float frac = scroll - px * t.scale;
		px %= ts.x;

		// far layers sink a bit when the bird climbs, ground (depth 1) stays put
		float bottom = win.y - l.y + cameraHeight * SWAY * (1.0f - 1.0f / l.depth);

		sf::Sprite s(t.tex[i]);
		s.setTextureRect(sf::IntRect({px, 0}, {w, static_cast<int>(ts.y)}));
		s.setScale({t.scale, t.scale});
		s.setPosition({-frac, bottom - ts.y * t.scale});
		window.draw(s);

		if (bottom >= win.y) continue;

		sf::RectangleShape fill({win.x, win.y - bottom});
		fill.setPosition({0.0f, bottom});
		fill.setFillColor(t.under[i]);
		window.draw(fill);
	}
}
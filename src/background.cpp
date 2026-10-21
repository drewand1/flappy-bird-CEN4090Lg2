#include <cmath>
#include <initializer_list>
#include "background.h"

constexpr float SCALE = 3.0f; // matches the ground/bird/pipe sprites
constexpr float SWAY = 0.1f;  // 0 turns the vertical parallax off

sf::Texture skyTex("rsc/textures/bg_sky.png");
sf::Texture cloudTex("rsc/textures/bg_clouds.png");
sf::Texture cityTex("rsc/textures/bg_city.png");
sf::Texture bushTex("rsc/textures/bg_bushes.png");
sf::Texture groundTex("rsc/textures/ground.png");

struct Layer {
	sf::Texture* tex;
	float depth; // 1 = pipe speed, 2 = half, 4 = quarter...
	float y;     // offset up from the bottom of the window
	sf::Color under;
};

// back to front. ground's offset is negative so most of it sits below the screen
Layer layers[] = {
	{&cloudTex, 8.0f, 66.0f, sf::Color(236, 248, 246)},
	{&cityTex, 4.0f, 34.0f, sf::Color(126, 196, 178)},
	{&bushTex, 2.0f, 12.0f, sf::Color(94, 186, 76)},
	{&groundTex, 1.0f, -27.0f, sf::Color(122, 68, 74)}
};

void initBackground() {
	for (sf::Texture* t : {&skyTex, &cloudTex, &cityTex, &bushTex, &groundTex})
		t->setSmooth(false);

	// sky just gets stretched, the rest wrap by sliding the texture rect
	for (Layer& l : layers)
		l.tex->setRepeated(true);
}

void drawBackground(sf::RenderWindow& window, float scrollDistance, float cameraHeight) {
	sf::Vector2u win = window.getSize();
	sf::Vector2u skySz = skyTex.getSize();

	sf::Sprite sky(skyTex);
	sky.setScale({static_cast<float>(win.x) / skySz.x, static_cast<float>(win.y) / skySz.y});
	window.draw(sky);

	int w = static_cast<int>(std::ceil(win.x / SCALE)) + 1; // +1 or the right edge shows a gap

	for (const Layer& l : layers) {
		sf::Vector2u ts = l.tex->getSize();
		float scroll = scrollDistance / l.depth;

		// rect only moves in whole texels (3 screen px) which looks choppy,
		// so nudge the sprite by the leftover
		int px = static_cast<int>(scroll / SCALE);
		float frac = scroll - px * SCALE;
		px %= ts.x;

		// far layers sink a bit when the bird climbs, ground (depth 1) stays put
		float bottom = win.y - l.y + cameraHeight * SWAY * (1.0f - 1.0f / l.depth);

		sf::Sprite s(*l.tex);
		s.setTextureRect(sf::IntRect({px, 0}, {w, static_cast<int>(ts.y)}));
		s.setScale({SCALE, SCALE});
		s.setPosition({-frac, bottom - ts.y * SCALE});
		window.draw(s);

		if (bottom >= win.y) continue;

		sf::RectangleShape fill({static_cast<float>(win.x), win.y - bottom});
		fill.setPosition({0.0f, bottom});
		fill.setFillColor(l.under);
		window.draw(fill);
	}
}

#include <cmath>
#include "background.h"
#include "gameconfig.h"

constexpr float SWAY = 0.1f;
constexpr float SKY_DEPTH = 16.0f;
struct Layer {
	float depth;
	float y;
};

//back to front: back, middle, front, ground. every theme uses the same spots
Layer layers[4] = {
	{8.0f, 66.0f},
	{4.0f, 34.0f},
	{2.0f, 12.0f},
	{1.0f, -27.0f}
};

const std::string files[4] = {"back.png", "middle.png", "front.png", "ground.png"};

Theme themes[THEME_COUNT] = {
	{"Classic", "classic", 3.0f},
	{"Mushroom Kingdom", "theme2", 3.0f},
	{"Minecraft", "theme3", 3.0f},
	{"Hallownest", "theme4", 1.0f}
};

void initBackground() {
	for (Theme& scene : themes) {
		std::string path = "rsc/textures/backgrounds/" + scene.folder + "/";

		if (!scene.sky.loadFromFile(path + "sky.png"))
			scene.sky = themes[0].sky;

		scene.sky.setSmooth(scene.sky.getSize().x > 64);

		for (int i = 0; i < 4; i++) {
			if (!scene.tex[i].loadFromFile(path + files[i]))
				continue;

			scene.has[i] = true;
			scene.tex[i].setSmooth(false);
		}
	}
}

void drawBackground(sf::RenderWindow& window, int theme, float scrollDistance, float cameraHeight) {
	const Theme& scene = themes[theme];
	sf::Vector2f screen(VIEW_WIDTH, VIEW_HEIGHT);

	sf::Vector2f skySheet(scene.sky.getSize());
	float skyStart = -std::fmod(scrollDistance / SKY_DEPTH, screen.x);

	for (float x = skyStart; x < screen.x; x += screen.x) {
		sf::Sprite cutout(scene.sky);
		cutout.setScale({screen.x / skySheet.x, screen.y / skySheet.y});
		cutout.setPosition({std::round(x), 0.0f});
		window.draw(cutout);
	}

	for (int i = 0; i < 4; i++) {
		if (!scene.has[i]) continue;

		const Layer& band = layers[i];
		sf::Vector2i bandSheet(scene.tex[i].getSize());
		float tileWidth = bandSheet.x * scene.scale;
		float start = -std::fmod(scrollDistance / band.depth, tileWidth);
		float bottom = screen.y - band.y + cameraHeight * SWAY * (1.0f - 1.0f / band.depth);

		for (float x = start; x < screen.x; x += tileWidth) {
			sf::Sprite cutout(scene.tex[i]);
			cutout.setScale({scene.scale, scene.scale});
			cutout.setPosition({std::round(x), bottom - bandSheet.y * scene.scale});
			window.draw(cutout);

			sf::Sprite filler(scene.tex[i], sf::IntRect({0, bandSheet.y - 1}, {bandSheet.x, 1}));
			filler.setScale({scene.scale, screen.y - bottom});
			filler.setPosition({std::round(x), bottom});
			window.draw(filler);
		}
	}
}
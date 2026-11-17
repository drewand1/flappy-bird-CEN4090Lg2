#include "uiinit.h"
#include "gamedraw.h"
#include "gamelogic.h"
#include "gamestatuses.h"

sf::Texture normalTex("rsc/textures/normal.png");
sf::Texture crushTex("rsc/textures/crush.png");
sf::Texture huntTex("rsc/textures/flaphunt.png");
sf::Texture dropperTex("rsc/textures/dropper.png");
sf::Texture dartsTex("rsc/textures/darts.png");
sf::Texture golfTex("rsc/textures/golf.png");

GameState* globalGameState = nullptr;

void playGamemode(cenui::Frame* frame) {
	resetGame(*globalGameState);

	if (frame->ID() == "NORMAL") {
		setGameStatus(*globalGameState, GameStatuses::playing);
	} else if (frame->ID() == "CRUSH") {
		setGameStatus(*globalGameState, GameStatuses::flappyCrush);
	} else if (frame->ID() == "GOLF") {
		setGameStatus(*globalGameState, GameStatuses::golf);
	} else if (frame->ID() == "DARTS") {
		setGameStatus(*globalGameState, GameStatuses::darts);
	} else if (frame->ID() == "DROPPER") {
		setGameStatus(*globalGameState, GameStatuses::dropper);
	} else if (frame->ID() == "DUCK HUNT") {
		setGameStatus(*globalGameState, GameStatuses::flapHunt);
	} else {
		setGameStatus(*globalGameState, GameStatuses::playing);
	}
}

cenui::Frame* makeGamemodeButton(std::string name, sf::Texture* tex) {
	static int buttonID = 0;
	cenui::Frame* newFrame = (new cenui::Frame(name))
		->position({{0.34f * (buttonID % 3), 0}, {0, 50.0f + 58 * (buttonID / 3)}})
		->size({{0.32f, 0}, {0, 50}})
		->outlineColor({230, 230, 230, 255})
		->texturePtr(tex)
		->bgColor({255, 255, 255, 255})
		->textColor({230, 230, 230, 255})
		->textOutlineColor({14, 14, 14, 255})
		->textOutlineThickness(3.0f)
		->text(name, gameFont, 14)
		->onClick(playGamemode);
	buttonID++;
	return newFrame;
}

void initFrameManager(cenui::FrameManager& fMgr) {
	cenui::Frame* menu = fMgr.root()->append(new cenui::Frame("gamemodeMenu"))
		->visible(false)
		->anchor({0.5f, 0.5f})
		->position({{0.5f, 0.5f}, {0, 0}})
		->size({{0, 0}, {500, 180}})
		->bgColor({14, 14, 14, 255})
		->outlineColor({230, 230, 230, 255})
		->padding({10.0f, 10.0f, 10.0f, 10.0f})
		->text("CHOOSE GAMEMODE", gameFont, 24u)
		->textColor({230, 230, 230, 255})
		->textYAlignment(cenui::YAlignment::TOP);
	
	menu->append(makeGamemodeButton("NORMAL", &normalTex));
	menu->append(makeGamemodeButton("CRUSH", &crushTex));
	menu->append(makeGamemodeButton("GOLF", &golfTex));
	menu->append(makeGamemodeButton("DARTS", &dartsTex));
	menu->append(makeGamemodeButton("DROPPER", &dropperTex));
	menu->append(makeGamemodeButton("DUCK HUNT", &huntTex));
}

void registerUIGameState(GameState* state) {
	globalGameState = state;
}
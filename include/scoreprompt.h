#ifndef SCOREPROMPT_H
#define SCOREPROMPT_H

#include "gamelogic.h"

// popup asking user to opt-in to submitting their score

void initScorePrompt(GameState& game);

void openScorePrompt(GameState& game);
void closeScorePrompt(GameState& game);
[[nodiscard]] bool scorePromptOpen();

void tickScorePrompt(GameState& game);

// Enter = confirm, Escape = back out.
void keyPressedScorePrompt(GameState& game, const sf::Event::KeyPressed* event);
void textEnteredScorePrompt(GameState& game, const sf::Event::TextEntered* event);

#endif

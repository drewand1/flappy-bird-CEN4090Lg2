#ifndef UIINIT_H
#define UIINIT_H

#include <cenui.h>
#include "gamelogic.h"

void initFrameManager(cenui::FrameManager& fMgr);
void registerUIGameState(GameState* state);

cenui::Frame* gamemodeMenuFrame();

#endif

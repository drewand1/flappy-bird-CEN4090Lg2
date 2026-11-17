#ifndef GAMESTATUSES_H
#define GAMESTATUSES_H

#include "gamelogic.h"
#include "statuses/menu.h"
#include "statuses/playing.h"
#include "statuses/gameover.h"
#include "statuses/birdselect.h"

namespace GameStatuses {	// Extra gamemodes
	extern GameStatus flappyCrush;
	extern GameStatus darts;
	extern GameStatus flapHunt;
	extern GameStatus golf;
	extern GameStatus dropper;
}



#endif
#pragma once

#include "Game.h"

// menu -> game_setup -> game_loop
//  ^-------^-- score_loop <-/
void tetro_game_setup(StateMachine&, Timer&);
void tetro_game_loop(StateMachine&, Timer&);
void tetro_score_loop(StateMachine&, Timer&);

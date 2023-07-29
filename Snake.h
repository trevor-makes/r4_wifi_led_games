#pragma once

#include "Game.h"

// menu -> game_setup -> game_loop -> death_setup
//  ^-------^-- score_loop <- death_loop <-/
void snake_game_setup(StateMachine&, Timer&);
void snake_game_loop(StateMachine&, Timer&);
void snake_death_setup(StateMachine&, Timer&);
void snake_death_loop(StateMachine&, Timer&);
void snake_score_loop(StateMachine&, Timer&);
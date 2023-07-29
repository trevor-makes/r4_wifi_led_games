#pragma once

#include "Game.h"

// menu -> game -> score
//  ^-------^-------/
extern const State tetro_menu_state;
extern const State tetro_game_state;
extern const State tetro_score_state;

#include "Snake.h"

// ... snake <-> tetro <-> snake ...
static constexpr const State& tetro_left_state = snake_menu_state;
static constexpr const State& tetro_right_state = snake_menu_state;
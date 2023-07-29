#pragma once

#include "Game.h"

// menu -> game -> death -> score
//  ^-------^----------------/
extern const State snake_menu_state;
extern const State snake_game_state;
extern const State snake_death_state;
extern const State snake_score_state;

#include "Tetro.h"

// ... tetro <-> snake <-> tetro ...
static constexpr const State& snake_left_state = tetro_menu_state;
static constexpr const State& snake_right_state = tetro_menu_state;
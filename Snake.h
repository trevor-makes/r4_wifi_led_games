#pragma once

#include "Game.h"

extern const State snake_menu_state;

#include "Tetro.h"

// ... tetro <-> snake <-> tetro ...
static constexpr const State& snake_left_state = tetro_menu_state;
static constexpr const State& snake_right_state = tetro_menu_state;
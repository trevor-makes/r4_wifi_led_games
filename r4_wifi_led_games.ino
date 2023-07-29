#include "Snake.h"
#include "Tetro.h"
#include "Frame.h"
#include "PlayStation.h"

#include <tuple>

void snake_menu_input(StateMachine&, Timer&);

State snake_menu = {
  .setup = snake_menu_state.setup,
  .loop = snake_menu_input,
};

void tetro_menu_input(StateMachine&, Timer&);

State tetro_menu = {
  .setup = tetro_menu_state.setup,
  .loop = tetro_menu_input,
};

template <const State& LEFT, const State& RIGHT, const State& START>
void menu_selection(StateMachine& state) {
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  if (pressed & (PlayStation.Left | PlayStation.L1)) {
    state.next(LEFT);
    return;
  } else if (pressed & (PlayStation.Right | PlayStation.R1)) {
    state.next(RIGHT);
    return;
  } else if (pressed & (PlayStation.Start | PlayStation.Cross)) {
    state.enter(START);
    return;
  }
}

void snake_menu_input(StateMachine& state, Timer& timer) {
  menu_selection<tetro_menu, tetro_menu, snake_game_state>(state);
  snake_menu_state.loop(state, timer);
}

void tetro_menu_input(StateMachine& state, Timer& timer) {
  menu_selection<snake_menu, snake_menu, tetro_game_state>(state);
  tetro_menu_state.loop(state, timer);
}

StateMachine state;

void setup() {
  PlayStation.begin();
  Frame.begin();

  state.enter(snake_menu);
}

void loop() {
  state.update();
}
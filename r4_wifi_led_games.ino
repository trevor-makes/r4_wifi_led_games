#include "Snake.h"
#include "Tetro.h"
#include "Frame.h"
#include "PlayStation.h"

StateMachine state;

void main_menu(StateMachine& state, Timer& timer) {
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  // TODO select game by left/right (arrows, shoulders), start with start
  if (pressed & PlayStation.Cross) {
    state.enter(snake_game_setup);
    return;
  } else if (pressed & PlayStation.Circle) {
    state.enter(tetro_game_setup);
    return;
  }

  if (timer.did_tick() == false) return;

  Frame.clear();
  // TODO add menu graphic/animation
  Frame.render();
}

void setup() {
  PlayStation.begin();
  Frame.begin();

  state.enter(main_menu);
}

void loop() {
  state.update();
}
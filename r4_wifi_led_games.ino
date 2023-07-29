#include "Snake.h"
#include "Tetro.h"
#include "Frame.h"
#include "PlayStation.h"

StateMachine state;

void setup() {
  PlayStation.begin();
  Frame.begin();

  state.enter(snake_menu_state);
}

void loop() {
  state.loop();
}
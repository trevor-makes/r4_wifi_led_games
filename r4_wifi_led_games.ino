#include "Snake.h"
#include "Tetro.h"
#include "Frame.h"
#include "PlayStation.h"

StateMachine state;

void setup() {
  PlayStation.begin();
  Frame.begin();

  state.next(menu_loop);
  //state.next(tetro_loop);
}

void loop() {
  state.update();
}
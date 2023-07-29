#pragma once

#include "Game.h"
#include "PlayStation.h"

template <const State& LEFT, const State& RIGHT, const State& START>
bool menu_selection(StateMachine& state) {
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  if (pressed & (PlayStation.Left | PlayStation.L1)) {
    state.next(LEFT);
    return true;
  } else if (pressed & (PlayStation.Right | PlayStation.R1)) {
    state.next(RIGHT);
    return true;
  } else if (pressed & PlayStation.Start) {
    state.enter(START);
    return true;
  }
  return false;
}
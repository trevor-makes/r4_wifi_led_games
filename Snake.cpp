#include "Snake.h"

static Head head;
static Tail tail;
static Apple apple;

static uint16_t score = 0;

constexpr uint8_t PERIOD_LIMIT = 60; // shortest frame period
constexpr uint8_t PERIOD_START = 150; // starting frame period

static unsigned long frame_period = PERIOD_START;

void reset_frame_period(Timer& timer) {
  frame_period = PERIOD_START;
  timer.set_period(frame_period);
  timer.reset();
}

void shrink_frame_period(Timer& timer) {
  if (frame_period > PERIOD_LIMIT) {
    // roughly multiply by ~0.96
    frame_period = (frame_period >> 6) + (frame_period >> 5) + (frame_period >> 4)
      + (frame_period >> 3) + (frame_period >> 2) + (frame_period >> 1);
    frame_period = max(frame_period, PERIOD_LIMIT);
  }
  timer.set_period(frame_period);
}

void death_loop(StateMachine& state, Timer& timer) {
  if (timer.did_tick() == false) return;

  shrink_frame_period(timer);

  if (tail.length() > 0) {
    tail.shrink();
    Frame.render();
  } else {
    state.next(menu_loop);
    return;
  }
}

void game_over(StateMachine& state, Timer& timer) {
  Frame.fill(true); // invert screen
  reset_frame_period(timer);
  state.next(death_loop);
}

void game_loop(StateMachine& state, Timer& timer) {
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  head.try_move(pressed & PlayStation.Left);
  head.try_move(pressed & PlayStation.Right);
  head.try_move(pressed & PlayStation.Up);
  head.try_move(pressed & PlayStation.Down);

  // Wait for next move
  if (timer.did_tick() == false) return;

  // Move forward; game over if head hits wall
  if (!head.update()) {
    state.next(game_over);
    return;
  }

  // Game over if head eats tail
  if (tail.overlaps(head)) {
    state.next(game_over);
    return;
  }

  tail.grow(head);

  // If an apple was eaten...
  if (apple.overlaps(head)) {
    ++score;
    tail.feed();
    shrink_frame_period(timer);
    apple.respawn(tail);
  }

  tail.try_shrink();

  Frame.render();
}

// TODO one-shot state could be setup/loop pattern
void restart(StateMachine& state, Timer& timer) {
  Frame.clear();

  randomSeed(micros());

  head.reset();
  tail.reset();
  tail.grow(head);
  apple.respawn(tail);
  score = 0;

  reset_frame_period(timer);
  state.next(game_loop);
}

void menu_loop(StateMachine& state, Timer& timer) {
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  if (pressed & PlayStation.Start) {
    state.next(restart);
    return;
  }

  if (timer.did_tick() == false) return;

  Frame.clear();
  Frame.plot_digit(0, 0, (score / 100) % 10, true);
  Frame.plot_digit(0, 4, (score / 10) % 10, true);
  Frame.plot_digit(0, 8, (score / 1) % 10, true);
  Frame.render();
}
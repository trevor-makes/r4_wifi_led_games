#include "Snake.h"
#include "Tetro.h"
#include "Frame.h"
#include "PlayStation.h"

void (*loop_ptr)(Timer& timer);
Timer g_timer;

void setup() {
  PlayStation.begin();
  Frame.begin();

  //show_menu();
  g_timer.set_period(500);
  loop_ptr = tetro_loop;
}

void loop() {
  loop_ptr(g_timer);
}

Head head;
Tail tail;
Apple apple;

uint16_t score = 0;

constexpr uint8_t PERIOD_LIMIT = 60; // shortest frame period
constexpr uint8_t PERIOD_START = 150; // starting frame period

void shrink_frame_period() {
  unsigned long frame_period = g_timer.get_period();
  if (frame_period > PERIOD_LIMIT) {
    // roughly multiply by ~0.96
    frame_period = (frame_period >> 6) + (frame_period >> 5) + (frame_period >> 4)
      + (frame_period >> 3) + (frame_period >> 2) + (frame_period >> 1);
    g_timer.set_period(frame_period);
  }
}

void restart() {
  Frame.clear();

  randomSeed(micros());

  head.reset();
  tail.reset();
  tail.grow(head);

  apple.respawn(tail);

  score = 0;
  g_timer.set_period(PERIOD_START);
  g_timer.reset();

  loop_ptr = game_loop;
}

void game_over() {
  Frame.clear(true); // invert screen
  g_timer.set_period(PERIOD_START);
  loop_ptr = death_loop;
}

void show_menu() {
  Frame.clear();
  Frame.plot_digit(0, 0, (score / 100) % 10, true);
  Frame.plot_digit(0, 4, (score / 10) % 10, true);
  Frame.plot_digit(0, 8, (score / 1) % 10, true);
  Frame.render();
  loop_ptr = menu_loop;
}

void menu_loop(Timer& timer) {
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  if (pressed & PlayStation.Start) { restart(); return; }

  //if (g_timer.did_tick() == false) return;

  // animate frame here
}

void death_loop(Timer& timer) {
  if (g_timer.did_tick() == false) return;

  shrink_frame_period();

  if (tail.length() > 0) {
    tail.shrink();
    Frame.render();
  } else {
    show_menu();
  }
}

void game_loop(Timer& timer) {
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  head.try_move(pressed & PlayStation.Left);
  head.try_move(pressed & PlayStation.Right);
  head.try_move(pressed & PlayStation.Up);
  head.try_move(pressed & PlayStation.Down);

  // Wait for next move
  if (g_timer.did_tick() == false) return;

  // Move forward; game over if head hits wall
  if (!head.update()) {
    game_over();
    return;
  }

  // Game over if head eats tail
  if (tail.overlaps(head)) {
    game_over();
    return;
  }

  tail.grow(head);

  // If an apple was eaten...
  if (apple.overlaps(head)) {
    ++score;
    tail.feed();
    shrink_frame_period();
    apple.respawn(tail);
  }

  tail.try_shrink();

  Frame.render();
}
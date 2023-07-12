#include "Snake.h"
#include "Frame.h"
#include "PlayStation.h"

void (*loop_ptr)();

void setup() {
  PlayStation.begin();
  Frame.begin();

  show_menu();
}

void loop() {
  loop_ptr();
}

Head head;
Tail tail;
Apple apple;

#define MIN_DELAY 60
uint8_t move_delay; // millis per move

unsigned long t_prev;

void restart() {
  Frame.clear();

  randomSeed(micros());

  head.reset();
  tail.reset();
  tail.grow(head);

  apple.respawn(tail);

  move_delay = 150;
  t_prev = millis();

  loop_ptr = game_loop;
}

void game_over() {
  move_delay = MIN_DELAY;
  loop_ptr = death_loop;
}

void show_menu() {
  Frame.clear();
  Frame.render();
  loop_ptr = menu_loop;
}

void menu_loop() {
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  if (pressed & PlayStation.Start) restart();
}

void death_loop() {
  unsigned long t_now = millis();
  if ((t_now - t_prev) < move_delay) return;
  t_prev = t_now;

  if (tail.length() > 0) {
    tail.shrink();
    Frame.render();
  } else {
    show_menu();
  }
}

void game_loop() {
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  head.try_move(pressed & PlayStation.Left);
  head.try_move(pressed & PlayStation.Right);
  head.try_move(pressed & PlayStation.Up);
  head.try_move(pressed & PlayStation.Down);

  // Wait for next move
  unsigned long t_now = millis();
  if ((t_now - t_prev) < move_delay) return;
  t_prev = t_now;

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
    tail.feed();
    if (move_delay > MIN_DELAY) {
      // roughly multiply by ~0.96
      move_delay = (move_delay >> 6) + (move_delay >> 5) + (move_delay >> 4)
        + (move_delay >> 3) + (move_delay >> 2) + (move_delay >> 1);
    }
    apple.respawn(tail);
  }

  tail.try_shrink();

  Frame.render();
}
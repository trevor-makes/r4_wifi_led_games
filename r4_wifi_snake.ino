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

uint16_t head_dir;
uint16_t prev_dir;
uint8_t head_row;
uint8_t head_col;

Tail tail;
Apple apple;

#define MIN_DELAY 60
uint8_t move_delay; // millis per move

unsigned long t_prev;

void restart() {
  Frame.clear();

  randomSeed(micros());

  head_dir = PlayStation.Right;
  prev_dir = head_dir;
  head_row = Frame.HEIGHT / 2;
  head_col = 1;
  tail.reset();
  tail.grow(head_row, head_col);

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
  if ((pressed & PlayStation.Left) && prev_dir != PlayStation.Right) head_dir = PlayStation.Left;
  if ((pressed & PlayStation.Right) && prev_dir != PlayStation.Left) head_dir = PlayStation.Right;
  if ((pressed & PlayStation.Up) && prev_dir != PlayStation.Down) head_dir = PlayStation.Up;
  if ((pressed & PlayStation.Down) && prev_dir != PlayStation.Up) head_dir = PlayStation.Down;

  // Wait for next move
  unsigned long t_now = millis();
  if ((t_now - t_prev) < move_delay) return;
  t_prev = t_now;

  switch (head_dir) {
    case PlayStation.Left:
      if (head_col == 0) {
        game_over();
        return;
      }
      head_col -= 1;
      break;
    case PlayStation.Right:
      if (head_col == Frame.WIDTH - 1) {
        game_over();
        return;
      }
      head_col += 1;
      break;
    case PlayStation.Up:
      if (head_row == 0) {
        game_over();
        return;
      }
      head_row -= 1;
      break;
    case PlayStation.Down:
      if (head_row == Frame.HEIGHT - 1) {
        game_over();
        return;
      }
      head_row += 1;
      break;
  }
  prev_dir = head_dir;

  // Game over if head eats tail
  if (tail.is_tail(head_row, head_col)) {
    game_over();
    return;
  }

  tail.grow(head_row, head_col);

  // If an apple was eaten...
  if (apple.is_apple(head_row, head_col)) {
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
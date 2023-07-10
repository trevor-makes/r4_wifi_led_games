#include "Frame.h"
#include "PlayStation.h"

void setup() {
  PlayStation.begin();
  Frame.begin();

  // TODO this sucks, figure out a decent way to seed random
  //randomSeed(analogRead(A0));
  restart();
}

uint16_t head_dir;
uint16_t prev_dir;
uint8_t head_row;
uint8_t head_col;

#define TAIL_MAX_LEN 80
uint8_t tail_rows[TAIL_MAX_LEN];
uint8_t tail_cols[TAIL_MAX_LEN];
uint8_t tail_index = 0;
uint8_t tail_length = 0;
uint8_t target_length = 6;

uint8_t apple_row;
uint8_t apple_col;

#define MIN_DELAY 60
uint8_t move_delay; // millis per move

unsigned long t_prev;

void restart() {
  Frame.clear();

  head_dir = PlayStation.Right;
  prev_dir = head_dir;
  head_row = Frame.HEIGHT / 2;
  head_col = 1;
  grow_tail(head_row, head_col);

  new_apple();

  move_delay = 150;
  t_prev = millis();
}

void game_over() {
  // TODO show score and option for restart
  for (;;) {}
}

void loop() {
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
      if (head_col == 0) game_over();
      head_col -= 1;
      break;
    case PlayStation.Right:
      if (head_col == Frame.WIDTH - 1) game_over();
      head_col += 1;
      break;
    case PlayStation.Up:
      if (head_row == 0) game_over();
      head_row -= 1;
      break;
    case PlayStation.Down:
      if (head_row == Frame.HEIGHT - 1) game_over();
      head_row += 1;
      break;
  }
  prev_dir = head_dir;

  // Game over if head eats tail
  if (is_tail(head_row, head_col)) game_over();

  grow_tail(head_row, head_col);

  // If an apple was eaten...
  if (head_row == apple_row && head_col == apple_col) {
    if (target_length < TAIL_MAX_LEN) {
      ++target_length;
    }
    if (move_delay > MIN_DELAY) {
      // roughly multiply by ~0.96
      move_delay = (move_delay >> 6) + (move_delay >> 5) + (move_delay >> 4)
        + (move_delay >> 3) + (move_delay >> 2) + (move_delay >> 1);
    }
    new_apple();
  }

  if (tail_length == target_length) shrink_tail();

  Frame.render();
}

void shrink_tail() {
  // Erase tail graphic
  Frame.plot(tail_rows[tail_index], tail_cols[tail_index], false);

  // Pop tail from ring buffer
  ++tail_index;
  if (tail_index == TAIL_MAX_LEN)
    tail_index = 0;
  --tail_length;
}

void grow_tail(uint8_t row, uint8_t col) {
  // Push head into ring buffer
  uint8_t head_index = tail_index + tail_length;
  if (head_index >= TAIL_MAX_LEN)
    head_index -= TAIL_MAX_LEN;
  tail_rows[head_index] = row;
  tail_cols[head_index] = col;
  ++tail_length;

  // Draw head graphic
  Frame.plot(row, col, true);
}

bool is_tail(uint8_t row, uint8_t col) {
  for (uint8_t i = 0; i < tail_length; ++i) {
    uint8_t index = tail_index + i;
    // Wrap index into circular buffer
    if (index >= TAIL_MAX_LEN) {
      index -= TAIL_MAX_LEN;
    }
    if (tail_rows[index] == row && tail_cols[index] == col) {
      return true;
    }
  }
  return false;
}

void new_apple() {
  // Get random position not on tail
  do {
    apple_row = random(Frame.HEIGHT);
    apple_col = random(Frame.WIDTH);
  } while (is_tail(apple_row, apple_col));

  // Draw apple at new position
  Frame.plot(apple_row, apple_col, true);
}
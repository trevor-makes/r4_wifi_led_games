#pragma once

#include "Frame.h"

class Tail {
private:
  static constexpr size_t MAX_LEN = 32; 
  uint8_t tail_rows[MAX_LEN];
  uint8_t tail_cols[MAX_LEN];
  uint8_t tail_index;
  uint8_t tail_length;
  uint8_t target_length;

public:
  void reset() {
    tail_index = 0;
    tail_length = 0;
    target_length = 6;
  }

  uint8_t length() const { return tail_length; }

  void shrink() {
    // Erase tail graphic
    Frame.plot(tail_rows[tail_index], tail_cols[tail_index], false);

    // Pop tail from ring buffer
    ++tail_index;
    if (tail_index == MAX_LEN)
      tail_index = 0;
    --tail_length;
  }

  void try_shrink() {
    if (tail_length == target_length) shrink();
  }

  void grow(uint8_t row, uint8_t col) {
    // Push head into ring buffer
    uint8_t head_index = tail_index + tail_length;
    if (head_index >= MAX_LEN) head_index -= MAX_LEN;
    tail_rows[head_index] = row;
    tail_cols[head_index] = col;
    ++tail_length;

    // Draw head graphic
    Frame.plot(row, col, true);
  }

  void feed() {
    if (target_length < MAX_LEN) {
      ++target_length;
    }
  }

  bool is_tail(uint8_t row, uint8_t col) const {
    for (uint8_t i = 0; i < tail_length; ++i) {
      uint8_t index = tail_index + i;
      // Wrap index into circular buffer
      if (index >= MAX_LEN) {
        index -= MAX_LEN;
      }
      if (tail_rows[index] == row && tail_cols[index] == col) {
        return true;
      }
    }
    return false;
  }
};

class Apple {
private:
  uint8_t apple_row;
  uint8_t apple_col;

public:
  void respawn(const Tail& tail) {
    // Get random position not on tail
    do {
      apple_row = random(Frame.HEIGHT);
      apple_col = random(Frame.WIDTH);
    } while (tail.is_tail(apple_row, apple_col));

    // Draw apple at new position
    Frame.plot(apple_row, apple_col, true);
  }

  bool is_apple(uint8_t row, uint8_t col) const {
    return row == apple_row && col == apple_col;
  }
};
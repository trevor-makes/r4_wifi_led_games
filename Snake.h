#pragma once

#include "Frame.h"
#include "PlayStation.h"
#include "Game.h"

void menu_loop(StateMachine&, Timer&);

class Head {
private:
  uint16_t head_dir;
  uint16_t prev_dir;
  uint8_t head_row;
  uint8_t head_col;

public:
  uint8_t row() const { return head_row; }
  uint8_t col() const { return head_col; }

  void reset() {
    head_dir = PlayStation.Right;
    prev_dir = head_dir;
    head_row = Frame.HEIGHT / 2;
    head_col = 1;
  }

  void try_move(uint16_t dir) {
    switch (dir) {
      case PlayStation.Left:
        if (prev_dir != PlayStation.Right) head_dir = dir;
        break;
      case PlayStation.Right:
        if (prev_dir != PlayStation.Left) head_dir = dir;
        break;
      case PlayStation.Up:
        if (prev_dir != PlayStation.Down) head_dir = dir;
        break;
      case PlayStation.Down:
        if (prev_dir != PlayStation.Up) head_dir = dir;
        break;
    }
  }

  bool update() {
    switch (head_dir) {
      case PlayStation.Left:
        if (head_col == 0) return false;
        head_col -= 1;
        break;
      case PlayStation.Right:
        if (head_col == Frame.WIDTH - 1) return false;
        head_col += 1;
        break;
      case PlayStation.Up:
        if (head_row == 0) return false;
        head_row -= 1;
        break;
      case PlayStation.Down:
        if (head_row == Frame.HEIGHT - 1) return false;
        head_row += 1;
        break;
    }
    prev_dir = head_dir;
    return true;
  }
};

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

  void grow(const Head& head) {
    grow(head.row(), head.col());
  }

  void feed() {
    if (target_length < MAX_LEN) {
      ++target_length;
    }
  }

  bool overlaps(uint8_t row, uint8_t col) const {
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

  bool overlaps(const Head& head) {
    return overlaps(head.row(), head.col());
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
    } while (tail.overlaps(apple_row, apple_col));

    // Draw apple at new position
    Frame.plot(apple_row, apple_col, true);
  }

  bool overlaps(uint8_t row, uint8_t col) const {
    return row == apple_row && col == apple_col;
  }

  bool overlaps(const Head& head) {
    return overlaps(head.row(), head.col());
  }
};
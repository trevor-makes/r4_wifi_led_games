#include "Snake.h"
#include "Frame.h"
#include "PlayStation.h"

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

void snake_death_loop(StateMachine& state, Timer& timer) {
  if (timer.did_tick() == false) return;

  shrink_frame_period(timer);

  if (tail.length() > 0) {
    tail.shrink();
    Frame.render();
  } else {
    state.next(snake_score_loop);
    return;
  }
}

void snake_death_setup(StateMachine& state, Timer& timer) {
  Frame.fill(true); // invert screen
  reset_frame_period(timer);
  state.next(snake_death_loop);
}

void snake_game_loop(StateMachine& state, Timer& timer) {
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
    state.next(snake_death_setup);
    return;
  }

  // Game over if head eats tail
  if (tail.overlaps(head)) {
    state.next(snake_death_setup);
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
void snake_game_setup(StateMachine& state, Timer& timer) {
  Frame.clear();

  randomSeed(micros());

  head.reset();
  tail.reset();
  tail.grow(head);
  apple.respawn(tail);
  score = 0;

  reset_frame_period(timer);
  state.next(snake_game_loop);
}

void snake_score_loop(StateMachine& state, Timer& timer) {
  PlayStation.update();
  uint16_t pressed = PlayStation.get_pressed();
  if (pressed & PlayStation.Select) {
    state.back(); // go back to the parent menu
    return;
  } else if (pressed & PlayStation.Start) {
    state.next(snake_game_setup);
    return;
  }

  if (timer.did_tick() == false) return;

  Frame.clear();
  Frame.plot_digit(0, 0, (score / 100) % 10, true);
  Frame.plot_digit(0, 4, (score / 10) % 10, true);
  Frame.plot_digit(0, 8, (score / 1) % 10, true);
  Frame.render();
}
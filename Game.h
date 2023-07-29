#pragma once

#include <Arduino.h>

#include <vector>

class Timer {
  unsigned long period_;
  unsigned long prev_;

public:
  void reset() { prev_ = millis(); }

  bool did_tick() {
    // Wait for next move
    unsigned long now = millis();
    if ((now - prev_) < period_) return false;
    prev_ = now;
    return true;
  }

  unsigned long get_period() const { return period_; }
  void set_period(unsigned long period) { period_ = period; }
};

class StateMachine {
  using StateFn = void(StateMachine&, Timer&);

  Timer timer_;
  std::vector<StateFn*> states_;

public:
  void enter(StateFn *state) {
    states_.push_back(state);
  }

  void back() {
    if (!states_.empty()) {
      states_.pop_back();
    }
  }

  void next(StateFn *state) {
    back(); // lose the current state
    enter(state);
  }

  void update() {
    if (!states_.empty()) states_.back()(*this, timer_);
  }
};
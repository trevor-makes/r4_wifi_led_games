#pragma once

#include <Arduino.h>

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
  StateFn *state_ = nullptr;

public:
  void next(StateFn *state) { state_ = state; }

  void update() {
    if (state_ != nullptr) state_(*this, timer_);
  }
};
#pragma once

#include "Globals.h"

class InputState
{
public:
  InputState() :
    _prev(0),
    _curr(0)
  {
  }

  void Set(uint8_t value)
  {
    _prev = _curr;
    _curr = value;
  }

  bool WasSet(uint8_t value)
  {
    return _prev & value > 0;
  }

  bool IsSet(uint8_t value)
  {
    return _curr & value > 0;
  }

  bool Down(uint8_t value)
  {
    return !WasSet(value) && IsSet(value);
  }

  bool Up(uint8_t value)
  {
    return WasSet(value) && !IsSet(value);
  }

private:
  uint8_t _prev;
  uint8_t _curr;
};

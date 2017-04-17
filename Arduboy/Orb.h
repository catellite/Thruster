#pragma once

#include "Vec2.h"

class Game;

class Orb
{
public:
  enum class State : uint8_t
  {
    unspecified,
    spawn,
    rest,
    active,
    dead,
    destroyed
  };

  Orb() :
    _state(State::unspecified) { }

  void SetState(Game* game, const State& state);
  const State& GetState() { return _state; }

  void Update(Game* game);
  void Draw(Game* game, const vec2s16& view);

  void SetPosition(const vec2s16& position) { _spawn = position; }
  const vec2s16& GetPosition() { return _position; }

  void Move(const vec2s16& offset) { _position += offset; }

private:
  State _state;
  uint16_t _stateTime;
  int8_t _frame;
  vec2s16 _spawn;
  vec2s16 _previous;
  vec2s16 _position;
  vec2s16 _accelleration;
};


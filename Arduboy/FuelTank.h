#pragma once

#include "Vec2.h"
#include "Globals.h"

class Game;

class Fuel
{
public:
  enum class State : uint8_t
  {
    unspecified,
    active,
    refueling,
    empty
  };

  Fuel() :
    _state(State::unspecified),
    _amount(0) { }

  void SetState(Game* game, const State& state);
  const State& GetState() { return _state; }

  const vec2s16& GetPosition() { return _position; }
  void SetPosition(const vec2s16& position) { _position = position; }

  bool CanRefuel() { return _amount > 0; }
  
  void Update(Game* game);
  void Draw(Game* game, const vec2s16& view);

private:
  State _state;
  vec2s16 _position;
  uint8_t _amount;
};


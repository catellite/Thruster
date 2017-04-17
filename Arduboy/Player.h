#pragma once

#include "Vec2.h"
#include "Particles.h"

class Game;

class Player
{
public:
  enum class State : uint8_t
  {
    unspecified,
    spawn,
    active,
    dead,
    destroyed
  };

  Player() :
    _state(State::unspecified),
    _frame(0),
    _thrust(false) { }

  void SetState(Game* game, const State& state);
  const State& GetState() { return _state; }

  void Update(Game* game);
  void Draw(Game* game);

  void SetSpawnPosition(const vec2s16& position) { _spawn = position; }
  const vec2s16& GetPosition() { return _position; }
  void Move(const vec2s16& offset) { _position += offset; }

  void RotateAnticlockwise() { _frame--; if (_frame == -1) _frame = 47; }
  void RotateClockwise() { _frame++; if (_frame == 48) _frame = 0; }

  void Thrust(bool value) { _thrust = value; }
  void Fire(Game* game);

  bool HasHit(Game* game, const vec2s16& topLeft, const vec2s16& bottomRight)
  {
    return _shots.HitTest(game, topLeft, bottomRight);
  }
  
private:
  int8_t SinTable(uint8_t index)
  {
    const int8_t values[] = {0, 64, 110, 127, 110, 64};
    return index < 6 ? values[index] : -values[index - 6];
  }

  int8_t CosTable(uint8_t index)
  {
    return SinTable((index + 3) % 12);
  }

private:
  State _state;
  uint16_t _stateTime;
  int8_t _frame;
  vec2s16 _spawn;
  vec2s16 _previous;
  vec2s16 _position;
  vec2s16 _accelleration;
  Particles<4> _shots;
  bool _thrust;
};


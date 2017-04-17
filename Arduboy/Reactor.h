#pragma once

#include "Vec2.h"
#include "Globals.h"
#include "Common.h"

class Game;

class Reactor
{
public:
  enum class State : uint8_t
  {
    unspecified,
    active,
    disabled,
    damaged,
    critical,
    destroyed
  };

  Reactor() :
    _state(State::unspecified),
	_stateTime(0),
	_damage(0) { }

  void SetState(Game* game, const State& state);
  const State& GetState() { return _state; }

  const vec2s16& GetPosition() { return _position; }
  void SetPosition(const vec2s16& position) { _position = position; }

  void Update(Game* game);
  void Draw(Game* game, const vec2s16& view);

  bool IsSolid(Game* game, const vec2s16& topLeft, const vec2s16& bottomRight)
  {
    return RectOverlap(topLeft, bottomRight, _position - vec2s16(64 * 7, 64 * 4), _position + vec2s16(64 * 7, 64 * 8));
  }

  void Damaged(Game* game)
  {
    if (_damage == 0)
    {
      _damage = 64;
    }
    else if (_damage > 255 - 32)
    {
      _damage = 255;
    }
    else
    {
      _damage += 32;
    }
  
    //game->GetDevice().Audio.Play(SoundEffect(4, 38 - 32, 0, 7, 0, 16, 0));
  }

private:
  State _state;
  uint16_t _stateTime;
  vec2s16 _position;
  uint8_t _damage;
};


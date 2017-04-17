#pragma once

#include "Vec2.h"
#include "Particles.h"
#include "Common.h"

class Game;
class Player;

class Enemy
{
public:
  enum class State : uint8_t
  {
    undefined,
    idle,
    shooting,
    dead
  };

  enum class Type : uint8_t
  {
    undefined,
    groundLeft,  
    groundRight,  
    ceilingLeft,  
    ceilingRight  
  };
  
  Enemy(uint8_t index) :
    _index(index),
    _type(Type::undefined),
    _state(State::undefined) { }

  void SetState(Game* game, const State& state);
  const State& GetState() { return _state; }

  const vec2s16& GetPosition() { return _position; }
  void SetPosition(const vec2s16& position, unsigned char tile)
  {
    _position = position;

    switch (tile & 0xf)
    {
      case 0x1: _type = Type::groundLeft; break;
      case 0x2: _type = Type::groundRight; break;
      case 0x9: _type = Type::ceilingLeft; break;
      case 0xa: _type = Type::ceilingRight; break;
    }
  }

  void SetType(Type type) { _type = type; };
  
  bool HasHit(Game* game, const vec2s16& topLeft, const vec2s16& bottomRight)
  {
    return _shots.HitTest(game, topLeft, bottomRight);
  }

  void Update(Game* game);
  void Draw(Game* game, const vec2s16& view);
  
  bool IsSolid(Game* game, const vec2s16& topLeft, const vec2s16& bottomRight)
  {
    if (_state != Enemy::State::dead)
    {
      return RectOverlap(topLeft, bottomRight, vec2s16(_position.x - 64 * 8, _position.y - 64 * 3), vec2s16(_position.x + 64 * 8, _position.y + 64 * 5));
  	}
  	else
  	{
      return false;
  	}
  }

private:
  uint8_t _index;
  State _state;
  Type _type;
  uint16_t _stateTime;
  vec2s16 _position;
  Particles<1> _shots;
};


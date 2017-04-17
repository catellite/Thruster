#pragma once

#include "Vec2.h"

class Game;

struct Particle
{
  Particle() :
    Life(0) { }

  //void Set(const vec2s16& position, const vec2s16& velocity, uint8_t life)
  //{
  //  _position = position;
  //  _velocity = velocity;
  //  _life = life;
  //}

  void Update(Game* game, bool useGravity, bool useResistance, bool clipAgainstMap);

  void Draw(Game* game, const vec2s16& view);

  bool HitTest(Game* game, const vec2s16& topLeft, const vec2s16& bottomRight);

  vec2s16 Position;
  vec2s16 Velocity;
  uint8_t Life;
};

template<uint8_t count>
class Particles
{
public:
  Particles() :
    _next(0) { }

  void Update(Game* game, bool useGravity, bool useResistance, bool clipAgainstMap)
  {
    for (uint8_t i = 0; i < count; i++)
    {
      _particle[i].Update(game, useGravity, useResistance, clipAgainstMap);
    }
  }

  void Draw(Game* game, const vec2s16& view)
  {
    for (uint8_t i = 0; i < count; i++)
    {
      _particle[i].Draw(game, view);
    }
  }

  void Explode(uint8_t num, const vec2s16& position, const vec2s16& range, int16_t velocity, uint8_t life)
  {
    for (uint8_t i = 0; i < num; i++)
    {
      _particle[_next].Position = vec2s16(position.x + random(-range.x, range.x), position.y + random(-range.y, range.y));
      _particle[_next].Velocity = vec2s16(velocity * random(-8, 8), velocity * random(-8, 8));
      _particle[_next].Life = random(life, 2 * life);

      _next++;

      if (_next == count)
      {
        _next = 0;
      }
    }
  }

  void Shoot(const vec2s16& position, const vec2s16& velocity, uint8_t life)
  {
    _particle[_next].Position = position;
    _particle[_next].Velocity = velocity;
    _particle[_next].Life = life;

    _next++;

    if (_next == count)
    {
      _next = 0;
    }
  }

  bool HitTest(Game* game, const vec2s16& topLeft, const vec2s16& bottomRight)
  {
    bool hit = false;

    for (uint8_t i = 0; i < count; i++)
    {
      hit |= _particle[i].HitTest(game, topLeft, bottomRight);
    }

    return hit;
  }

  void KillAll()
  {
    for (uint8_t i = 0; i < count; i++)
    {
      _particle[i].Life = 0;
    }
  }

protected:
  uint8_t _next;
  Particle _particle[count];
};


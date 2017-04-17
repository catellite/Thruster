#pragma once

#include "Vec2.h"

class Game;

class Stars
{
  static const uint8_t Count = 8;
  
  struct Star
  {
    Star() :
      Age(0) { }
      
    vec2u8 Position;
    uint8_t Age;
  };
  
public:
  Stars() { }
  
  void Update();
  void Draw(Game* game, const vec2s16& view);

private:
  Star _stars[Count];
};


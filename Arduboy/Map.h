#pragma once

#include "Vec2.h"

class Game;

class Map
{
public:
  Map();

  void SetLevel(uint8_t index) { _index = index; }

  enum Sprite : uint8_t
  {
    player,
    orb,
    fuel,
    reactor,
    enemyLeft,
    enemyRight
  };

  vec2s16 GetStart(Sprite sprite);

  bool IsSolid(Game* game, const vec2s16& position);
  bool IsSolid(Game* game, const vec2s16& topLeft, const vec2s16& bottomRight);

  bool HasEscaped(const vec2s16& position) { return TileAt(position) == 0xf; }
  
  void Draw(Game* game, const vec2s16& view);

private:
  const unsigned char* MapAddress();
  unsigned char TileAt(const vec2s16& position);

  int8_t GetBitmapIndex(unsigned char tile)
  {
    const int8_t index[] = { -1, 3, 6, 1, -1, 4, 5, -1, -1, 7, 10, 2, -1, 8, 9, 0 };
    return index[tile];
  }

  vec2s16 GetTilePosition(unsigned char chr, unsigned char msk, unsigned char* value = 0, int8_t index = 0);
  
private:
  uint8_t _index;
};


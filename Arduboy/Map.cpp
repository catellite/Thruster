#include "Map.h"
#include "Game.h"
#include "Data.h"

Map::Map() :
  _index(0) { }

vec2s16 Map::GetStart(Sprite sprite)
{
  const unsigned char* addr = MapAddress() + 256 + 2 * (uint8_t)sprite;
  unsigned char lo = pgm_read_byte(addr);
  unsigned char hi = pgm_read_byte(addr + 1);
  int16_t offset = (hi == 0x10) ? 0 : 16;
  return vec2s16(((int16_t)(lo & 0x0f) * 32 + offset + 8) * 64, ((int16_t)(lo >> 4) * 16 + 8) * 64);
}

void Map::Draw(Game* game, const vec2s16& view)
{
  int16_t px = view.x / 64;
  int16_t py = view.y / 64;

  uint16_t x0 = px / 16;
  uint16_t y0 = py / 16;

  uint8_t xf = px & 0xf;
  uint8_t yf = py & 0xf;

  const unsigned char* map = MapAddress();

  for (uint8_t y = 0; y < 5; y++)
  {
    for (uint8_t x = 0; x < 9; x++)
    {
      uint16_t tx = (32 + x0 + x - 4) & 0x1f;
      uint16_t ty = (32 + y0 + y - 2) & 0x1f;

      unsigned char pair = pgm_read_byte(map + ty * 16 + (tx / 2));
      unsigned char tile = tx & 1 ? pair & 0xf : pair >> 4;

      if (tile == 0x4 || tile == 0x7)
      {
        // is a door

        unsigned char door = (pair >> 4) & 0x3;

        uint8_t openness = game->GetDoorOffset(door);
        
        if (tile == 0x4)
        {
          DrawSprite(game->GetDevice(), x * 16 - xf - openness, y * 16 - yf, tileData, 0, 1, MaskMode::mask);
        }
        else
        {
          DrawSprite(game->GetDevice(), x * 16 - xf + openness, y * 16 - yf, tileData, 0, 1, MaskMode::mask);
        }
      }
      else
      {
        // regular ole background tile

        int8_t index = GetBitmapIndex(tile);
        if (index != -1)
        {
          DrawSprite(game->GetDevice(), x * 16 - xf, y * 16 - yf, tileData, 2 * index, 2 * index + 1, MaskMode::mask);
        }
      }
    }
  }
}

bool Map::IsSolid(Game* game, const vec2s16& position)
{
  int16_t px = position.x / 64;
  int16_t py = position.y / 64;
  
  uint8_t xf = px & 0xf;
  uint8_t yf = py & 0xf;
  
  unsigned char tile = TileAt(position);
  
  switch (tile)
  {
    case 0x0: return false;                               // blank
    case 0x1: return (yf - 8) * 2 > (15 - xf);            // low left slope
    case 0x2: return (yf - 8) * 2 > xf;                   // low right slope
    case 0x3: return yf >= 8;                             // low half solid
    case 0x4: return (15 - xf) >= game->GetDoorOffset(0); // solid left door
    case 0x5: return yf * 2 > (15 - xf);                  // high left slope
    case 0x6: return yf * 2 > xf;                         // high right slope
    case 0x7: return xf >= game->GetDoorOffset(1);        // solid right door
    case 0x9: return ((15 - yf) - 8) * 2 > (15 - xf);     // inverted low left slope
    case 0xa: return ((15 - yf) - 8) * 2 > xf;            // inverted low right slope
    case 0xb: return yf <= 8;                             // inverted low half solid
    case 0xd: return (15 - yf) * 2 > (15 - xf);           // inverted high left slope
    case 0xe: return (15 - yf) * 2 > xf;                  // inverted high right slope
    case 0xf: return true;                                // solid
    default: return true;                                 // undefined
  }
}

bool Map::IsSolid(Game* game, const vec2s16& topLeft, const vec2s16& bottomRight)
{
  // fast and dirty corner test 
  return (IsSolid(game, vec2s16(topLeft.x, topLeft.y)) ||
    IsSolid(game, vec2s16(bottomRight.x, topLeft.y)) ||
    IsSolid(game, vec2s16(topLeft.x, bottomRight.y)) ||
    IsSolid(game, vec2s16(bottomRight.x, bottomRight.y)));
}

const unsigned char* Map::MapAddress()
{
  return &missionsData[_index * (256 + 16)];
}

unsigned char Map::TileAt(const vec2s16& position)
{
  int16_t px = position.x / 64;
  int16_t py = position.y / 64;

  int16_t x0 = px / 16;
  int16_t y0 = py / 16;

  unsigned char pair = pgm_read_byte(MapAddress() + y0 * 16 + (x0 / 2));

  return x0 & 1 ? pair & 0xf : pair >> 4;
}

vec2s16 Map::GetTilePosition(unsigned char chr, unsigned char msk, unsigned char* value, int8_t index)
{
  const unsigned char* map = MapAddress();

  for (int8_t ty = 0; ty < 32; ty++)
  {
    for (int8_t tx = 0; tx < 32; tx++)
    {
      unsigned char tile = pgm_read_byte(map + ty * 32 + tx);
      
      if ((tile & msk) == chr)
      {
        if (index == 0)
        {
          if (value != 0)
          {
            *value = tile;
          }
          
          return vec2s16(((int16_t)tx * 16 + 8) * 64, ((int16_t)ty * 16 + 8) * 64); 
        }
        else
        {
          index--;
        }
      }
    }
  }

  return vec2s16();
}


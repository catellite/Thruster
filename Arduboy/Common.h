#pragma once

#include "Device.h"
#include "Data.h"
#include "Vec2.h"

#define DEBUG

#if defined(DEBUG)
static void InitDebug()
{
#if !defined WIN32
  Serial.begin(9600);
#endif
}

static void DebugText(const char* text)
{
#if !defined WIN32
  Serial.println(text);
#else
  OutputDebugStringA(text);
#endif
}
#else
static void InitDebug() { }
static void DebugText(const char*) { }
#endif

static uint16_t StrLen(const unsigned char* str)
{
    const unsigned char* end;
    for (end = str; pgm_read_byte(end); ++end);
    return end - str;
}

static uint32_t Sqrt(uint32_t value)
{
  uint32_t op  = value;
  uint32_t result = 0;
  uint32_t one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type

  // "one" starts at the highest power of four <= than the argument.
  while (one > op)
  {
    one >>= 2;
  }

  while (one != 0)
  {
    if (op >= result + one)
    {
      op = op - (result + one);
      result = result +  2 * one;
    }
    result >>= 1;
    one >>= 2;
  }
  
  return result;
}

enum class Color : uint8_t { white, black };
enum class HorizontalAlign : uint8_t { left, centre, right };
enum class VerticalAlign : uint8_t { top, middle, bottom };
enum class MaskMode : uint8_t { set, unset, mask };

static void BlitBitmap(Arduboy& device, const uint8_t *bitmap)
{
  unsigned char* sBuffer = device.getBuffer();

  for (int i = 0; i < 1024; i++)
  {
    sBuffer[i] = pgm_read_byte(bitmap++);
  }
}

static void DrawBitmap(Arduboy& device, int16_t x, int16_t y, const uint8_t *bitmap, const uint8_t *mask, int8_t w, int8_t h, MaskMode mode)
{
  // no need to draw at all of we're offscreen
  if (x + w <= 0 || x > WIDTH - 1 || y + h <= 0 || y > HEIGHT - 1)
    return;

  unsigned char* sBuffer = device.getBuffer();
  
  if (bitmap == 0)
    return;

  // xOffset technically doesn't need to be 16 bit but the math operations
  // are measurably faster if it is
  int16_t xOffset, ofs;
  int8_t yOffset = abs(y) % 8;
  int8_t sRow = y / 8;
  uint8_t loop_h, start_h, rendered_width;

  if (y < 0 && yOffset > 0)
  {
    sRow--;
    yOffset = 8 - yOffset;
  }

  // if the left side of the render is offscreen skip those loops
  if (x < 0)
  {
    xOffset = abs(x);
  }
  else
  {
    xOffset = 0;
  }

  // if the right side of the render is offscreen skip those loops
  if (x + w > WIDTH - 1)
  {
    rendered_width = ((WIDTH - x) - xOffset);
  }
  else
  {
    rendered_width = (w - xOffset);
  }

  // if the top side of the render is offscreen skip those loops
  if (sRow < -1)
  {
    start_h = abs(sRow) - 1;
  }
  else
  {
    start_h = 0;
  }

  loop_h = h / 8 + (h % 8 > 0 ? 1 : 0); // divide, then round up

  // if (sRow + loop_h - 1 > (HEIGHT/8)-1)
  if (sRow + loop_h > (HEIGHT / 8))
  {
    loop_h = (HEIGHT / 8) - sRow;
  }

  // prepare variables for loops later so we can compare with 0
  // instead of comparing two variables
  loop_h -= start_h;

  sRow += start_h;
  ofs = (sRow * WIDTH) + x + xOffset;
  uint8_t *bofs = (uint8_t *)bitmap + (start_h * w) + xOffset;
  uint8_t *mask_ofs;
  if (mask != 0)
    mask_ofs = (uint8_t *)mask + (start_h * w) + xOffset;
  uint8_t data;

  uint8_t mul_amt = 1 << yOffset;
  uint16_t mask_data;
  uint16_t bitmap_data;

  switch (mode)
  {
    case MaskMode::set:
    {
      for (uint8_t a = 0; a < loop_h; a++)
      {
        for (uint8_t iCol = 0; iCol < rendered_width; iCol++)
        {
          bitmap_data = pgm_read_byte(bofs) * mul_amt;
          
          if (sRow >= 0)
          {
            sBuffer[ofs] |= (uint8_t)(bitmap_data);
          }
          
          if (yOffset != 0 && sRow < 7)
          {
            sBuffer[ofs + WIDTH] |= (*((unsigned char*)(&bitmap_data) + 1));
          }
          
          ofs++;
          bofs++;
        }
        
        sRow++;
        bofs += w - rendered_width;
        ofs += WIDTH - rendered_width;
      }
    }
    break;

    case MaskMode::unset:
    {
      for (uint8_t a = 0; a < loop_h; a++)
      {
        for (uint8_t iCol = 0; iCol < rendered_width; iCol++)
        {
          bitmap_data = pgm_read_byte(bofs) * mul_amt;
          
          if (sRow >= 0)
          {
            sBuffer[ofs]  &= ~(uint8_t)(bitmap_data);
          }
          
          if (yOffset != 0 && sRow < 7)
          {
            sBuffer[ofs + WIDTH] &= ~(*((unsigned char*)(&bitmap_data) + 1));
          }
          
          ofs++;
          bofs++;
        }
        
        sRow++;
        bofs += w - rendered_width;
        ofs += WIDTH - rendered_width;
      }
    }
    break;

    case MaskMode::mask:
    {
      for (uint8_t a = 0; a < loop_h; a++)
      {
        for (uint8_t iCol = 0; iCol < rendered_width; iCol++)
        {
          // NOTE: you might think in the yOffset==0 case that this results
          // in more effort, but in all my testing the compiler was forcing
          // 16-bit math to happen here anyways, so this isn't actually
          // compiling to more code than it otherwise would. If the offset
          // is 0 the high part of the word will just never be used.

          // load data and bit shift
          // mask needs to be bit flipped
          mask_data = ~(pgm_read_byte(mask_ofs) * mul_amt);
          bitmap_data = pgm_read_byte(bofs) * mul_amt;

          if (sRow >= 0)
          {
            data = sBuffer[ofs];
            data &= (uint8_t)(mask_data);
            data |= (uint8_t)(bitmap_data);
            sBuffer[ofs] = data;
          }
          if (yOffset != 0 && sRow < 7)
          {
            data = sBuffer[ofs + WIDTH];
            data &= (*((unsigned char*)(&mask_data) + 1));
            data |= (*((unsigned char*)(&bitmap_data) + 1));
            sBuffer[ofs + WIDTH] = data;
          }
          ofs++;
          mask_ofs++;
          bofs++;
        }
        sRow++;
        bofs += w - rendered_width;
        mask_ofs += w - rendered_width;
        ofs += WIDTH - rendered_width;
      }
    }
    break;
  }
}

static void DrawSprite(Arduboy& device, int16_t x, int16_t y, const unsigned char* bitmap, uint8_t sprite, uint8_t mask, MaskMode mode)
{
  uint16_t size = (bitmap[0] * (bitmap[1] / 8 + (bitmap[1] % 8 == 0 ? 0 : 1)));
  
  DrawBitmap(device, x, y, &bitmap[2] + sprite * size, &bitmap[2] + mask * size, bitmap[0], bitmap[1], mode);
}

static void DrawChar(Arduboy& device, int16_t x, int16_t y, char chr, Color color = Color::white)
{
  int8_t index = -1;
  if (chr >= '0' && chr <= '9') index = chr - '0';
  else if (chr >= 'A' && chr <= 'Z') index = 10 + chr - 'A';
  else if (chr == '.') index = 10 + 26;
  //else if (chr == ',') index = 10 + 26 + 1;
  //else if (chr == '(') index = 10 + 26 + 2;
  //else if (chr == ')') index = 10 + 26 + 3;
  //else if (chr == ')') index = 10 + 26 + 4;
  //else if (chr == '?') index = 10 + 26 + 5;

  if (index != -1)
  {
    DrawSprite(device, x, y, fontData, index, 0, color == Color::white ? MaskMode::set : color == Color::black ? MaskMode::unset : MaskMode::set);
  }
}

static void DrawText(Arduboy& device, int16_t x, int16_t y, const unsigned char* str, HorizontalAlign h = HorizontalAlign::centre, VerticalAlign v = VerticalAlign::middle, Color color = Color::white, bool pgm = true)
{
  int16_t cx = x;
  int16_t cy = y;

  if (h != HorizontalAlign::left)
  {
    int16_t size = StrLen(str) * (fontData[0] + 1) - 1;
    cx -= h == HorizontalAlign::right ? size : size / 2;
  }

  if (v != VerticalAlign::top)
  {
    cy -= v == VerticalAlign::bottom ? fontData[1] : fontData[1] / 2;
  }

  unsigned char ch = pgm ? pgm_read_byte(str) : *str;

  while (ch != '\0')
  {
    DrawChar(device, cx, cy, ch, color);
    cx += (int16_t)fontData[0] + 1;
    str++;
    ch = pgm ? pgm_read_byte(str) : *str;
  }
}

static void DrawNumber(Arduboy& device, int16_t x, int16_t y, uint16_t value, HorizontalAlign h = HorizontalAlign::centre, VerticalAlign v = VerticalAlign::middle, Color color = Color::white)
{
  unsigned char str[6];
  str[4] = '0';
  str[5] = '\0';
  unsigned char* digit = value == 0 ? &str[4] : &str[5];

  while (value > 0)
  {
    digit--;
    *digit = '0' + (value % 10);
    value /= 10;
  }

  DrawText(device, x, y, digit, h, v, color, false);
}

static bool PointInsideRect(const vec2s16& position, const vec2s16& topLeft, const vec2s16& bottomRight)
{
	return position.x >= topLeft.x && position.x <= bottomRight.x && position.y >= topLeft.y && position.y <= bottomRight.y;
}

static bool RectOverlap(const vec2s16& topLeft1, const vec2s16& bottomRight1, const vec2s16& topLeft2, const vec2s16& bottomRight2)
{
	return ((topLeft1.x <= bottomRight2.x) && (bottomRight1.x >= topLeft2.x) && (topLeft1.y <= bottomRight2.y) && (bottomRight1.y >= topLeft2.y));
}

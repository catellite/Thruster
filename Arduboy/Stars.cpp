#include "Stars.h"
#include "Game.h"

void Stars::Update()
{
  for (uint8_t i = 0; i < Count; i++)
  {
    if (_stars[i].Age == 0)
    {
      // refresh

      _stars[i].Position.x = random();
      _stars[i].Position.y = random();
      _stars[i].Age = random() & 0x7f;
    }

    _stars[i].Age--;
  }
}

void Stars::Draw(Game* game, const vec2s16& view)
{
  for (uint8_t i = 0; i < Count; i++)
  {
    // twinkle every 4 frames

    if ((_stars[i].Age & 0x3) > 0)
    {
      // reverse scroll direction and slow down

      vec2u8 screenPos = _stars[i].Position + vec2u8(-view.x / 512, -view.y / 512);

      // mask to the visible screen area

      game->GetDevice().drawPixel(screenPos.x & 0x7f, screenPos.y & 0x3f, WHITE);
    }
  }
}


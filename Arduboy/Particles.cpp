#include "Particles.h"
#include "Game.h"

void Particle::Update(Game* game, bool useGravity, bool useResistance, bool clipAgainstMap)
{
  if (Life > 0)
  {
    if (clipAgainstMap)
    {
      if (game->GetMap().IsSolid(game, Position))
      {
        Life = 0;
      }
    }
  }

  if (Life > 0)
  {
    Position += Velocity;

    if (game->GetSettings() & Game::Settings::gravity && useGravity)
    {
      Velocity.y += gravity;
    }

#if defined (HAS_RESISTANCE)
    if (useResistance)
    {
      int16_t rounding = resistance - 1;
      Velocity.x -= (Velocity.x >= 0 ? Velocity.x + rounding : Velocity.x - rounding) / resistance;
      Velocity.y -= (Velocity.y >= 0 ? Velocity.y + rounding : Velocity.y - rounding) / resistance;
    }
#endif

    Life--;
  }
}

void Particle::Draw(Game* game, const vec2s16& view)
{
  if (Life > 0)
  {
    int16_t x = Position.x > view.x ? (int16_t)((WIDTH / 2) + (Position.x - view.x) / 64) + 1 : (int16_t)((WIDTH / 2) + (Position.x - view.x) / 64);
    int16_t y = Position.y > view.y ? (int16_t)((HEIGHT / 2) + (Position.y - view.y) / 64) + 1 : (int16_t)((HEIGHT / 2) + (Position.y - view.y) / 64);

    game->GetDevice().drawPixel(x, y, WHITE);
  }
}

bool Particle::HitTest(Game* game, const vec2s16& topLeft, const vec2s16& bottomRight)
{
  if (Life > 0)
  {
    if (PointInsideRect(Position, topLeft, bottomRight))
    {
      Life = 0;
	  return true;
    }
  }

  return false;
}

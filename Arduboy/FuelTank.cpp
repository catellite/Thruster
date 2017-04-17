#include "FuelTank.h"
#include "Game.h"
#include "Data.h"

void Fuel::SetState(Game* game, const State& state)
{
  if (_state != state)
  {
    switch (state)
    {
      case State::active:
      {
        _amount = 250;
      }
      break;
    }

    _state = state;
  }
}

void Fuel::Update(Game* game)
{
  if (_state == State::refueling)
  {
    if (_amount > 0)
    {
      _amount -= refuel;
    }
    else
    {
      SetState(game, State::empty);
    }
  }
}

void Fuel::Draw(Game* game, const vec2s16& view)
{
  if (_amount > 0)
  {
    int16_t x = _position.x > view.x ? (int16_t)((WIDTH / 2) + (_position.x - view.x) / 64) + 1 : (int16_t)((WIDTH / 2) + (_position.x - view.x) / 64);
    int16_t y = _position.y > view.y ? (int16_t)((HEIGHT / 2) + (_position.y - view.y) / 64) + 1 : (int16_t)((HEIGHT / 2) + (_position.y - view.y) / 64);

    if (_state == State::active || game->GetDevice().frameCount & 1)
    {
      DrawSprite(game->GetDevice(), x, y, spriteData, 28, 29, MaskMode::mask);
    }
  }
}


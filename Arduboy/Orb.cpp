#include "Orb.h"
#include "Game.h"
#include "Data.h"

void Orb::SetState(Game* game, const State& state)
{
  if (_state != state)
  {
    switch (state)
    {
    case State::spawn:
    {
      _position = _spawn;
      _previous = _spawn;
      _accelleration = vec2s16();
    }
    break;
    }

    _state = state;
    _stateTime = 0;
  }
}

void Orb::Update(Game* game)
{
  switch (_state)
  {
    case State::spawn:
    {
      if (_stateTime > 32)
      {
        SetState(game, State::rest);
      }
    }
    break;

    case State::rest:
    {
    }
    break;

    case State::active:
    {
      vec2s16 temp = _position;
      vec2s16 vel = (_position - _previous) + _accelleration;
      
      vel.y += game->GetSettings() & Game::Settings::gravity ? gravity : 0;

#if defined (HAS_RESISTANCE)
      int16_t rounding = resistance - 1;
      vel.x -= (vel.x >= 0 ? vel.x + rounding : vel.x - rounding) / resistance;
      vel.y -= (vel.y >= 0 ? vel.y + rounding : vel.y - rounding) / resistance;
#endif
  
      _position = _position + vel;
      _previous = temp;
    }
    break;

    case State::dead:
    {
      if (_stateTime > orbRespawn)
      {
        SetState(game, State::spawn);
      }
    }
    break;
  }

  _stateTime++;
}

void Orb::Draw(Game* game, const vec2s16& view)
{
  switch (_state)
  {
    case State::spawn:
    {
      if (_stateTime & 1)
      {
        int16_t x = _position.x > view.x ? (int16_t)((WIDTH / 2) + (_position.x - view.x) / 64) + 1 : (int16_t)((WIDTH / 2) + (_position.x - view.x) / 64);
        int16_t y = _position.y > view.y ? (int16_t)((HEIGHT / 2) + (_position.y - view.y) / 64) + 1 : (int16_t)((HEIGHT / 2) + (_position.y - view.y) / 64);

        DrawSprite(game->GetDevice(), x - 3, y - 4, spriteData, 24, 25, MaskMode::mask);
      }
    }
    break;

    case State::rest:
    case State::active:
    {
      int16_t x = _position.x > view.x ? (int16_t)((WIDTH / 2) + (_position.x - view.x) / 64) + 1 : (int16_t)((WIDTH / 2) + (_position.x - view.x) / 64);
      int16_t y = _position.y > view.y ? (int16_t)((HEIGHT / 2) + (_position.y - view.y) / 64) + 1 : (int16_t)((HEIGHT / 2) + (_position.y - view.y) / 64);

      DrawSprite(game->GetDevice(), x - 3, y - 4, spriteData, 24, 25, MaskMode::mask);
    }
    break;

    case State::dead:
    {
    }
    break;
  }

  int16_t x = _spawn.x > view.x ? (int16_t)((WIDTH / 2) + (_spawn.x - view.x) / 64) + 1 : (int16_t)((WIDTH / 2) + (_spawn.x - view.x) / 64);
  int16_t y = _spawn.y > view.y ? (int16_t)((HEIGHT / 2) + (_spawn.y - view.y) / 64) + 1 : (int16_t)((HEIGHT / 2) + (_spawn.y - view.y) / 64);

  DrawSprite(game->GetDevice(), x - 3, y, spriteData, 26, 27, MaskMode::mask);
}

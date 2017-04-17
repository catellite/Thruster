#include "Reactor.h"
#include "Game.h"
#include "Data.h"

void Reactor::SetState(Game* game, const State& state)
{
  if (_state != state)
  {
    switch (state)
    {
      case State::active:
      {
        _damage = 0;
      }
      break;
    }

    _state = state;
    _stateTime = 0;
  }
}

void Reactor::Update(Game* game)
{
  switch (_state)
  {
    case State::active:
	  {
        if (_damage > 0)
        {
          SetState(game, State::disabled);
	    }
	  }
	  break;

    case State::disabled:
    {
      if (_damage == 255)
      {
        SetState(game, State::damaged);
      }
      else if (_damage > 0)
      {
        _damage--;

        if (_damage == 0)
        {
          SetState(game, State::active);
        }
      }
    }
    break;

    case State::damaged:
    {
      if (_stateTime >= 192)
      {
        SetState(game, State::critical);
      }
    }
    break;

    case State::critical:
    {
      if (_stateTime >= 64)
      {
        SetState(game, State::destroyed);
      }
    }
    break;
  }

#if defined WIN32
  char temp[256];
  sprintf_s(temp, "%d\n", _damage);
  DebugText(temp);
#endif

  _stateTime++;
}

void Reactor::Draw(Game* game, const vec2s16& view)
{
  int16_t x = _position.x > view.x ? (int16_t)((WIDTH / 2) + (_position.x - view.x) / 64) + 1 : (int16_t)((WIDTH / 2) + (_position.x - view.x) / 64);
  int16_t y = _position.y > view.y ? (int16_t)((HEIGHT / 2) + (_position.y - view.y) / 64) + 1 : (int16_t)((HEIGHT / 2) + (_position.y - view.y) / 64);

  bool visible = _state == State::active || _state == State::disabled || (_state == State::damaged && _stateTime & 0x8) || (_state == State::critical && _stateTime & 0x4);

  if (visible)
  {
    DrawSprite(game->GetDevice(), x - 8, y - 8, tileData, 2 * 11, 2 * 11 + 1, MaskMode::mask);

    if (_state == State::active)
    {
      uint8_t frame = (game->GetDevice().frameCount / 2) & 0x3;

      game->GetDevice().drawPixel(x + 4, y - 4 - frame, WHITE);
    }
  }
}





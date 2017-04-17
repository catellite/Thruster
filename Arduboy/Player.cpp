#include "Player.h"
#include "Game.h"
#include "Data.h"

void Player::SetState(Game* game, const State& state)
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
        _frame = 0;
      }
      break;

      case State::active:
      {
      }
      break;

      case State::dead:
      {
        //game->GetDevice().Audio.Play(SoundEffect(24, -1, 0, 7, 0, 48, 0));

        _thrust = false;
      }
      break;

      case State::destroyed:
      {
      }
      break;
    }

    _state = state;
    _stateTime = 0;
  }
}

void Player::Update(Game* game)
{
  switch (_state)
  {
  case State::spawn:
  {
    if (_stateTime > 16)
    {
      SetState(game, State::active);
    }
  }
  break;

  case State::active:
  {
    _shots.Update(game, false, false, true);

    if (_thrust)
    {
      _accelleration.x = SinTable(_frame / 4) / 16;
      _accelleration.y = gravity - CosTable(_frame / 4) / 16;

      //game->GetDevice().Audio.Play(SoundEffect(1, 48, 0, 7, 0, 1, 0));
    }

    vec2s16 temp = _position;
    vec2s16 vel = (_position - _previous) + _accelleration;

#if defined (HAS_RESISTANCE)
    int16_t rounding = resistance - 1;
    vel.x -= (vel.x >= 0 ? vel.x + rounding : vel.x - rounding) / resistance;
    vel.y -= (vel.y >= 0 ? vel.y + rounding : vel.y - rounding) / resistance;
#endif

    _position = _position + vel;
    _accelleration = vec2s16(0, game->GetSettings() & Game::Settings::gravity ? gravity : 0);
    _previous = temp;
  }
  break;

  case State::dead:
  {
    if (_stateTime > 96)
    {
      SetState(game, State::destroyed);
    }
  }
  break;

  case State::destroyed:
  {

  }
  break;
  }

  _stateTime++;
}

void Player::Draw(Game* game)
{
  switch (_state)
  {
  case State::spawn:
  {
    if (_stateTime & 1)
    {
      DrawSprite(game->GetDevice(), (WIDTH / 2) - 3, (HEIGHT / 2) - 4, spriteData, 0, 1, MaskMode::mask);
    }
  }
  break;

  case State::active:
  {
    _shots.Draw(game, _position);
    DrawSprite(game->GetDevice(), (WIDTH / 2) - 3, (HEIGHT / 2) - 4, spriteData, 2 * (_frame / 4), 2 * (_frame / 4) + 1, MaskMode::mask);

    if (_thrust && _stateTime & 1)
    {
      DrawSprite(game->GetDevice(), (WIDTH / 2) - 3, (HEIGHT / 2) - 4, spriteData, 30 + 2 * ((_frame / 4) & 0x3), 30 + 2 * ((_frame / 4) % 4) + 1, MaskMode::mask);
    }
  }
  break;

  case State::dead:
  {

  }
  break;

  case State::destroyed:
  {

  }
  break;
  }
}

void Player::Fire(Game* game)
{
  //game->GetDevice().Audio.Play(SoundEffect(128, -16, 0, 7, 0, 4, 0));
  _shots.Shoot(_position, vec2s16(2 * SinTable(_frame / 4), -2 * CosTable(_frame / 4)), 32);
}



#include "Enemy.h"
#include "Data.h"
#include "Game.h"

void Enemy::SetState(Game* game, const State& state)
{
  if (_state != state)
  {
    switch (state)
    {
    case State::undefined:
      {
        _shots.KillAll();
      }
      break;

    case State::dead:
      {
        //game->GetDevice().Audio.Play(SoundEffect(20, -1, 0, 7, 0, 16, 1));
      }
      break;
    }

    _state = state;
    _stateTime = 0;
  }
}

void Enemy::Update(Game* game)
{
  switch (_state)
  {
    case State::idle:
    {

    }
    break;

    case State::shooting:
    {
      if (game->GetSettings() & Game::Settings::enemies)
      {
        _shots.Update(game, false, false, true);

        if (_stateTime >= 128)
        {
          // skip early frames

          if (_stateTime % 64 == 0)
          {
            // time to shoot

            uint8_t index = _stateTime / 64;

            if (index % 2 == _index)
            {
              // alternate thru enemies

              vec2s16 dir = game->GetPlayer().GetPosition() - _position;

              // TODO: only shoot if near
              // TODO: shot test agains map

              // length in multiples of world units
              int16_t length = (int16_t)(Sqrt((int32_t)dir.x * (int32_t)dir.x + (int32_t)dir.y * (int32_t)dir.y) / 64);

              vec2s16 normalized = dir / length;

              _shots.Shoot(_position, normalized * 2, 64);
            }
          }
        }
      }
    }
    break;
  }

  _stateTime++;
}

void Enemy::Draw(Game* game, const vec2s16& view)
{
  if (_state == State::shooting)
  {
    _shots.Draw(game, view);
  }

  if (_state != State::dead)
  {
    int16_t x = _position.x > view.x ? (int16_t)((WIDTH / 2) + (_position.x - view.x) / 64) + 1 : (int16_t)((WIDTH / 2) + (_position.x - view.x) / 64);
    int16_t y = _position.y > view.y ? (int16_t)((HEIGHT / 2) + (_position.y - view.y) / 64) + 1 : (int16_t)((HEIGHT / 2) + (_position.y - view.y) / 64);

    switch (_type)
    {
      case Type::groundLeft: DrawSprite(game->GetDevice(), x - 8, y - 3, enemyData, 0, 1, MaskMode::mask); break;
      case Type::groundRight: DrawSprite(game->GetDevice(), x - 8, y - 3, enemyData, 2, 3, MaskMode::mask); break;
    }
  }
}


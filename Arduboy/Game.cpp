#include "Game.h"
#include "Map.h"

#if defined WIN32
#include <windows.h>
#include <stdio.h>
#endif

void Game::SetState(const State& state)
{
  if (_state != state)
  {
    // state enter

    switch (state)
    {
      case State::splash:
      {
        BlitBitmap(GetDevice(), splashData);
        LoadScores();
      }
      break;

      case State::titleReplay:
      {
        Start();

        InitLevel(startLevel);
        StartLevel();
      }
      break;

      case State::gameIntro:
      {
        StartLevel();
      }
      break;

      case State::gameRetry:
      {
        StartLevel();
      }
      break;

      case State::newHighScore:
      {
        _item = 0;
        _name[0] = _name[1] = _name[2] = 'A';
      }
      break;
    }

    // state exit

    switch (_state)
    {
    case State::gameExploding:
    {
      GetDevice().invert(false);
    }
    break;

    case State::gameCompleted:
    {
      _level++;
      InitLevel(_level);
    }
    break;
    }

    _state = state;
    _stateTime = 0;
  }
}

void Game::Update()
{
  uint8_t input = GetDevice().buttonsState();
    
  switch (_state)
  {
    case State::unspecified:
    {
      // get things started

      SetState(State::splash);
    }
    break;
    
    case State::splash:
    {
      // check for cheat code

      bool cheatActive = false;

      const uint8_t cheatCode[] = { UP_BUTTON, UP_BUTTON, DOWN_BUTTON, DOWN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON, LEFT_BUTTON, RIGHT_BUTTON, B_BUTTON, A_BUTTON };

      if (input != 0 && _input == 0)
      {
        // check last button against cheat code

        if (input == cheatCode[_item])
        {
          // so far so good...

          _item++;

          if (_item == 10)
          {
            // boom, let's go

            _item = 0;
            cheatActive = true;
          }
        }
        else
        {
          // better luck next time

          _item = 0;
        }
      }

      if (cheatActive)
      {
        // go to the trainer

        SetState(State::trainer);
      }
      else
      {
        if ((!(_input & A_BUTTON) && input & A_BUTTON) || _stateTime == splashTime)
        {
          // or start the game

          SetState(State::titleReplay);
        }
      }
    }
    break;

    case State::trainer:
    {
      // do the menu

      if (input & UP_BUTTON && !(_input & UP_BUTTON))
      {
        // up

        if (_item > 0)
        {
          _item--;
        }
      }
      else if (input & DOWN_BUTTON && !(_input & DOWN_BUTTON))
      {
        // down

        if (_item < 4)
        {
          _item++;
        }
      }

      // get the cheat flag 

      uint8_t flag = 1 << _item;

      if (input & RIGHT_BUTTON && !(input & LEFT_BUTTON))
      {
        // set it

        _settings |= flag;
      }
      else if (input & LEFT_BUTTON && !(input & RIGHT_BUTTON))
      {
        // clear it

        _settings &= ~flag;
      }

      if (input & A_BUTTON && !(_input & A_BUTTON))
      {
        // we're done

        SetState(State::titleReplay);
      }

      // do the pretty stars

      _stars.Update();
    }
    break;

    case State::titleReplay:
    {
      // do we need to?

      if (_stateTime == 256)
      {
        // end of replay

        SetState(State::titleScores);
      }

      // get replay data for this frame

      uint8_t replay = 0;
      uint8_t index = 0;
      uint8_t length = pgm_read_byte(replayData);

      while (pgm_read_byte(replayData + 1 + 2 * index) < _stateTime && index < length)
      {
        replay = pgm_read_byte(replayData + 2 + 2 * index);
        index++;
      }

      // fake the game

      UpdateGame(_replay, replay);

      _replay = replay;

      // check real inputs

      if (!(_input & A_BUTTON) && input & A_BUTTON)
      {
        // exit replay

        _fuel = 1000;
        _score = 0;
        _displayScore = 0;
        _lives = 3;

        _level = 0;

        InitLevel(_level);
        SetState(State::gameIntro);
      }
    }
    break;

    case State::titleScores:
    {
      // twinkle

      _stars.Update();

      // are we done?

      if ((!(_input & A_BUTTON) && input & A_BUTTON) || _stateTime > 256)
      {
        SetState(State::titleReplay);
      }
    }
    break;

    case State::gameIntro:
    {
      // sparkly

      _stars.Update();

      // go?

      if ((!(_input & A_BUTTON) && input & A_BUTTON) || _stateTime > levelTime)
      {
        SetState(State::gameActive);
      }
    }
    break;

    case State::gameActive:
    {
#if defined WIN32 && defined RECORD_GAMEPLAY
      char str[256];
      if (_stateTime < 256)
      {
        if (input != _input)
        {
          sprintf_s(str, "0x%02x, 0x%02x, \n", _stateTime, input);
          OutputDebugStringA(str);
        }
      }
      else
      {
        OutputDebugStringA("Finished recording\n");
        SetState(State::gameOver);
      }
#endif
      // real game update

      UpdateGame(_input, input);

      if (_player.GetState() == Player::State::destroyed)
      {
        // player is dead so stop enemies firing

        _enemy0.SetState(this, Enemy::State::undefined);
        _enemy1.SetState(this, Enemy::State::undefined);

        // update lives accordingly

        if (!(_settings & Settings::infiniteLives))
        {
          _lives--;
        }

        // reset or game over?

        SetState(_lives > 0 ? State::gameIntro : State::gameOver);
      }
    }
    break;

    case State::gameExploding:
    {
      // keep updating

      UpdateGame(_input, input);

      if (_stateTime >= 64)
      {
        // that's enough

        SetState(_lives > 0 ? State::gameIntro : State::gameOver);
      }
    }
    break;

    case State::gameOutro:
    {
      if ((!(_input & A_BUTTON) && input & A_BUTTON) || _stateTime > outroTime)
      {
        // move on

        SetState(State::gameCompleted);
      }

      _stars.Update();
    }
    break;

    case State::gameCompleted:
    {
      if ((!(_input & A_BUTTON) && input & A_BUTTON) || _stateTime > completedTime)
      {
        // nothing to see here

        SetState(State::gameIntro);
      }

      _stars.Update();
    }
    break;

    case State::gameRetry:
    {
      if ((!(_input & A_BUTTON) && input & A_BUTTON) || _stateTime > gameOverTime)
      {
        // go again

        SetState(State::gameIntro);
      }

      _stars.Update();
    }
    break;

    case State::gameOver:
    {
      if ((!(_input & A_BUTTON) && input & A_BUTTON) || _stateTime > gameOverTime)
      {
        // how did we do?

        if (_score > _scores[4].Value)
        {
          // we have a winner!

          SetState(State::newHighScore);
        }
        else
        {
          // better luck next time

          SetState(State::titleReplay);
        }
      }

      // keep things ticking

      _stars.Update();
      _player.Update(this);
      _orb.Update(this);
      Constrain();
      _exploder.Update(this, true, true, false);
    }
    break;

    case State::newHighScore:
    {
      // enter your name

      if ((!(_input & A_BUTTON) && input & A_BUTTON))
      {
        if (_item == 2)
        {
          // only 3 chars so that's you done

          uint8_t place = 4;
          
          // commit the score, but where?

          while (place > 0 && _score > _scores[place - 1].Value)
          {
            _scores[place].Value = _scores[place - 1].Value;
            
            _scores[place].Name[0] = _scores[place - 1].Name[0];
            _scores[place].Name[1] = _scores[place - 1].Name[1];
            _scores[place].Name[2] = _scores[place - 1].Name[2];

            place--;
          }

          // here

          _scores[place].Value = _score;

          _scores[place].Name[0] = _name[0];
          _scores[place].Name[1] = _name[1];
          _scores[place].Name[2] = _name[2];

          // burn it

          SaveScores();

          // we're outta here

          SetState(State::titleScores);
        }
        else
        {
          // move along

          _item++;
        }
      }

      // pick a letter

      if ((!(_input & UP_BUTTON) && input & UP_BUTTON))
      {
        if (_name[_item] == 'A')
        {
          _name[_item] = '.';
        }
        else if (_name[_item] == '.')
        {
          _name[_item] = '9';
        }
        else if (_name[_item] == '0')
        {
          _name[_item] = 'Z';
        }
        else
        {
          _name[_item]--;
        }
      }

      if ((!(_input & DOWN_BUTTON) && input & DOWN_BUTTON))
      {
        if (_name[_item] == 'Z')
        {
          _name[_item] = '0';
        }
        else if (_name[_item] == '9')
        {
          _name[_item] = '.';
        }
        else if (_name[_item] == '.')
        {
          _name[_item] = 'A';
        }
        else
        {
          _name[_item]++;
        }
      }

      // so shiny!

      _stars.Update();
    }
    break;
  }

  // score keeps moving

  if (_displayScore < _score)
  {
    _displayScore += 100;
    
    if (_displayScore > _score)
    {
      _displayScore = _score;
    }
  }

  // store the last button state, could manage the buttons better for sure...

  _input = input;
}

void Game::Render()
{
  if (_state == State::splash)
  {
    if (_stateTime > splashTime - 64)
    {
      // dirty pixelated wipe screen buffer

      for (int i = 0; i < 2; i++)
      {
        // wipe 2 pixels per 8x8 block per frame to get it done quicker

        int16_t tick = 2 * _stateTime + i;
        int16_t rndPix = tick * 139;

        int16_t x = rndPix & 0x7;
        int16_t y = (rndPix >> 4) & 0x7;

        for (int h = 0; h < 64; h += 8)
        {
          for (int w = 0; w < 128; w += 8)
          {
            // erase pixel

            GetDevice().drawPixel(w + x, h + y, BLACK);
          }
        }
      }
    }
  }
  else
  {
    GetDevice().clear();

    DrawStars();

    if (_state == State::trainer)
    {
      // show the trainer

      DrawTrainerMenu();
    }
    else if (_state == State::titleScores)
    {
      // show some scores

      DrawHighScores();
      DrawPressAToStart();
    }
    else
    {
      // show the game

      DrawBackground();
      DrawForeground();

      if (_state == State::titleReplay)
      {
        DrawPressAToStart();
      }
      else
      {
        DrawStats();
      }

      if (_state == State::gameIntro)
      {
        DrawMissionStart();
      }
      else if (_state == State::gameCompleted)
      {
        // we done, outta here

        GetDevice().fillRect(0, 32 - 3, 128, 7, BLACK);
        DrawText(GetDevice(), 64, 32, missionCompleteString, HorizontalAlign::centre, VerticalAlign::middle);
      }
      else if (_state == State::gameRetry)
      {
        // have another go

        GetDevice().fillRect(0, 32 - 3, 128, 7, BLACK);
        DrawText(GetDevice(), 64, 32, missionAbortedString, HorizontalAlign::centre, VerticalAlign::middle);
      }
      else if (_state == State::gameOver)
      {
        // you're done buddy

        GetDevice().fillRect(0, 32 - 3, 128, 7, BLACK);
        DrawText(GetDevice(), 64, 32, gameOverString, HorizontalAlign::centre, VerticalAlign::middle);
      }
      else if (_state == State::newHighScore)
      {
        // who's your daddy?

        GetDevice().fillRect(0, 32 - 3, 128, 7, BLACK);

        for (uint8_t ch = 0; ch < 3; ch++)
        {
          if (ch != _item || _stateTime & 0x8)
          {
            DrawChar(GetDevice(), 54 + ch * 7, 32 - 2, _name[ch]);
          }
        }
      }
    }
  }

  // show me what you got

  GetDevice().display();

  // done

  GetDevice().idle();
}

void Game::Start()
{
  _fuel = 1000;
  _score = 0;
  _displayScore = 0;
  _lives = 3;
}

void Game::InitLevel(uint8_t level)
{
  _map.SetLevel(level);

  _player.SetSpawnPosition(_map.GetStart(Map::Sprite::player));
  _orb.SetPosition(_map.GetStart(Map::Sprite::orb));

  _refuel.SetPosition(_map.GetStart(Map::Sprite::fuel));
  _refuel.SetState(this, Fuel::State::active);

  _reactor.SetPosition(_map.GetStart(Map::Sprite::reactor));
  _reactor.SetState(this, Reactor::State::active);

  _enemy0.SetPosition(_map.GetStart(Map::Sprite::enemyLeft), 0);
  _enemy1.SetPosition(_map.GetStart(Map::Sprite::enemyRight), 1);
}

void Game::StartLevel()
{
  _player.SetSpawnPosition(_map.GetStart(Map::Sprite::player));
  _orb.SetPosition(_map.GetStart(Map::Sprite::orb));

  _player.SetState(this, Player::State::spawn);
  _orb.SetState(this, Orb::State::spawn);

  //_enemies[i].SetState(this, _level > 1 ? Enemy::State::shooting : Enemy::State::idle);
  _enemy0.SetState(this, Enemy::State::shooting);
  _enemy1.SetState(this, Enemy::State::shooting);

  _attached = false;
}

void Game::UpdateGame(uint8_t prev, uint8_t input)
{
  if (_player.GetState() == Player::State::active)
  {
    // make a move

    if (input & RIGHT_BUTTON && !(input & LEFT_BUTTON))
    {
      _player.RotateClockwise();
    }
    else if (input & LEFT_BUTTON && !(input & RIGHT_BUTTON))
    {
      _player.RotateAnticlockwise();
    }

    // reset thrust then update

    _player.Thrust(false);

    if (input & A_BUTTON)
    {
      if (_fuel > 0)
      {
        _player.Thrust(true);

        if (!(_settings & Settings::infiniteFuel))
        {
          _fuel--;
        }
      }
    }

    // shooting

    if (input & B_BUTTON && !(prev & B_BUTTON))
    {
      _player.Fire(this);
    }

    // pick up check

    if (input & UP_BUTTON)
    {
      // need to do some crude distance checks

      vec2s16 diff = _orb.GetPosition() - _player.GetPosition();

      // orb distance in multiples of 256 units

      int16_t length = (int16_t)(Sqrt((int32_t)diff.x * (int32_t)diff.x + (int32_t)diff.y * (int32_t)diff.y) / 64);

      if (length < 16)
      {
        // we're close enough to pick up the orb

        _attached = true;
      }

      diff = _refuel.GetPosition() - _player.GetPosition();

      // fuel distance in multiples of 256 units

      length = (int16_t)(Sqrt((int32_t)diff.x * (int32_t)diff.x + (int32_t)diff.y * (int32_t)diff.y) / 64);

      if (length < 16)
      {
        // can refuel

        if (_refuel.GetState() != Fuel::State::empty)
        {
          _refuel.SetState(this, Fuel::State::refueling);
          _fuel += refuel;
        }
      }
      else
      {
        // just to confirm it's active

        _refuel.SetState(this, Fuel::State::active);
      }
    }
    else
    {
      if (_refuel.GetState() != Fuel::State::empty)
      {
        // amd again, could be nicer, but just to confirm it's active

        _refuel.SetState(this, Fuel::State::active);
      }
    }

    if (input & DOWN_BUTTON)
    {
      // drop it

      _attached = false;
    }

    if (_settings & Settings::collisions)
    {
      // do some bounds checks on a player box

      vec2s16 halfSize = vec2s16(64 * 3, 64 * 3);
      vec2s16 topLeft = _player.GetPosition() - halfSize;
      vec2s16 bottomRight = _player.GetPosition() + halfSize;

      bool collision = false;

      if (_map.IsSolid(this, topLeft, bottomRight))
      {
        collision = true;
      }
      else if (_enemy0.IsSolid(this, topLeft, bottomRight))
      {
        collision = true;
      }
      else if (_enemy1.IsSolid(this, topLeft, bottomRight))
      {
        collision = true;
      }
      else if (_reactor.IsSolid(this, topLeft, bottomRight))
      {
        collision = true;
      }

  	  if (collision)
      {
        // yup, we hit something

      	_player.SetState(this, Player::State::dead);
      	_exploder.Explode(12, _player.GetPosition(), vec2s16(256, 256), 12, 64);
      	_attached = false;
      }
	  }

    // or have we left the planet?

    if (_map.HasEscaped(_player.GetPosition()))
    {
      if (_attached)
      {
        // good job!

        _score += 2000;
        SetState(State::gameOutro);
      }
      else
      {
        // unlucky

        SetState(State::gameRetry);
      }
    }
  }

  // check that free-wheeling orb for collisions

  if (_orb.GetState() == Orb::State::active)
  {
    if (_settings & Settings::collisions)
    {
      vec2s16 halfSize = vec2s16(64 * 3, 64 * 3);
  	  vec2s16 topLeft = _orb.GetPosition() - halfSize;
  	  vec2s16 bottomRight = _orb.GetPosition() + halfSize;
  
      bool collision = false;

      if (_map.IsSolid(this, topLeft, bottomRight))
      {
        collision = true;
      }
      else if (_enemy0.IsSolid(this, topLeft, bottomRight))
      {
        collision = true;
      }
      else if (_enemy1.IsSolid(this, topLeft, bottomRight))
      {
        collision = true;
      }
      else if (_reactor.IsSolid(this, topLeft, bottomRight))
      {
        collision = true;
      }

      if (collision)
      {
        // not so good

        _orb.SetState(this, Orb::State::dead);
        _exploder.Explode(8, _orb.GetPosition(), vec2s16(256, 256), 12, 32);
        _attached = false;
  	  }
    }
  }

  if (_state != State::gameExploding && _reactor.GetState() == Reactor::State::destroyed)
  {
    // bad reactor, bad for you

    SetState(State::gameExploding);
  }

  if (_reactor.GetState() == Reactor::State::active)
  {
    // reactor active, enemies active

    if (_enemy0.GetState() == Enemy::State::idle)
    {
      //_enemy0.SetState(this, _level > 1 ? Enemy::State::shooting : Enemy::State::idle);
      _enemy0.SetState(this, Enemy::State::shooting);
    }

    if (_enemy1.GetState() == Enemy::State::idle)
    {
      //_enemy1.SetState(this, _level > 1 ? Enemy::State::shooting : Enemy::State::idle);
      _enemy1.SetState(this, Enemy::State::shooting);
    }
  }
  else if (_reactor.GetState() == Reactor::State::disabled)
  {
    // no power so stop the baddies

    if (_enemy0.GetState() == Enemy::State::shooting)
    {
      _enemy0.SetState(this, Enemy::State::idle);
    }

    if (_enemy1.GetState() == Enemy::State::shooting)
    {
      _enemy1.SetState(this, Enemy::State::idle);
    }
  }

  // anything shot?

  if (_reactor.GetState() != Reactor::State::damaged && _reactor.GetState() != Reactor::State::critical)
  {
    // you didn't blow it yet

    vec2s16 halfSize = vec2s16(64 * 16, 64 * 16);

    if (_player.HasHit(this, _reactor.GetPosition() - halfSize, _reactor.GetPosition() + halfSize))
    {
      // but you hit it

      _reactor.Damaged(this);
      _exploder.Explode(4, _reactor.GetPosition(), vec2s16(4 * 64, 4 * 64), 12, 32);
    }
  }

  if (_enemy0.GetState() != Enemy::State::dead)
  {
    if (_player.HasHit(this, _enemy0.GetPosition() - vec2s16(64 * 8, 64 * 3), _enemy0.GetPosition() + vec2s16(64 * 8, 64 * 5)))
    {
      // that's a hit 

      _enemy0.SetState(this, Enemy::State::dead);
      _exploder.Explode(8, _enemy0.GetPosition(), vec2s16(4 * 64, 4 * 64), 12, 32);
      _score += 750;
    }
  }

  if (_enemy1.GetState() != Enemy::State::dead)
  {
    if (_player.HasHit(this, _enemy0.GetPosition() - vec2s16(64 * 8, 64 * 3), _enemy1.GetPosition() + vec2s16(64 * 8, 64 * 5)))
    {
      // that's a hit 

      _enemy1.SetState(this, Enemy::State::dead);
      _exploder.Explode(8, _enemy1.GetPosition(), vec2s16(4 * 64, 4 * 64), 12, 32);
      _score += 750;
    }
  }

  if (_player.GetState() == Player::State::active)
  {
    if ((_enemy0.GetState() == Enemy::State::shooting && _enemy0.HasHit(this, _player.GetPosition() - vec2s16(64 * 3, 64 * 3), _player.GetPosition() + vec2s16(64 * 3, 64 * 3))) ||
      (_enemy1.GetState() == Enemy::State::shooting && _enemy1.HasHit(this, _player.GetPosition() - vec2s16(64 * 3, 64 * 3), _player.GetPosition() + vec2s16(64 * 3, 64 * 3))))
    {
      _player.SetState(this, Player::State::dead);
      _exploder.Explode(12, _player.GetPosition(), vec2s16(256, 256), 16, 64);
      _attached = false;
    }
  }

  _stars.Update();
  _player.Update(this);
  _orb.Update(this);

  // constrain orb and player positions

  Constrain();

  // fuel is good

  if (_refuel.GetState() == Fuel::State::refueling)
  {
    _refuel.Update(this);

    if (_refuel.GetState() == Fuel::State::empty)
    {
      // fuel empty

      _score += 300;
    }
  }

  _reactor.Update(this);

  _enemy0.Update(this);
  _enemy1.Update(this);

  _exploder.Update(this, true, true, false);
}

void Game::LoadScores()
{
  // get scores from eeprom

  uint16_t addr = 0;

  uint16_t id = EEPROM.read(addr++) << 8;
  id |= EEPROM.read(addr++);

  if (id != gameId)
  {
    // invalid save data, reset scores

    for (int entry = 0; entry < 5; entry++)
    {
      _scores[entry].Value = (5 - entry) * 1000;
      _scores[entry].Name[0] = 'B';
      _scores[entry].Name[1] = 'G';
      _scores[entry].Name[2] = 'S';
      _scores[entry].Name[3] = '\0';
    }
  }
  else
  {
    for (uint8_t entry = 0; entry < 5; entry++)
    {
      int16_t y = 15 + 7 * entry;

      _scores[entry].Value = EEPROM.read(addr++) << 8;
      _scores[entry].Value |= EEPROM.read(addr++);

      _scores[entry].Name[0] = EEPROM.read(addr++);
      _scores[entry].Name[1] = EEPROM.read(addr++);
      _scores[entry].Name[2] = EEPROM.read(addr++);
      _scores[entry].Name[3] = '\0';
    }
  }
}

void Game::SaveScores()
{
  // save scores to eeprom

  uint16_t addr = 0;

  EEPROM.write(addr++, (gameId >> 8) & 0xff);
  EEPROM.write(addr++, gameId & 0xff);

  for (uint8_t entry = 0; entry < 5; entry++)
  {
    uint16_t score = (entry + 1) * 1000;
    EEPROM.write(addr++, (_scores[entry].Value >> 8) & 0xff);
    EEPROM.write(addr++, _scores[entry].Value & 0xff);
    EEPROM.write(addr++, _scores[entry].Name[0]);
    EEPROM.write(addr++, _scores[entry].Name[1]);
    EEPROM.write(addr++, _scores[entry].Name[2]);
  }
}

void Game::Constrain()
{
  if (_attached)
  {
    vec2s16 dir = _orb.GetPosition() - _player.GetPosition();
  
    // length in multiples of world units

    int16_t length = (int16_t)(Sqrt((int32_t)dir.x * (int32_t)dir.x + (int32_t)dir.y * (int32_t)dir.y) / 64);

    if (_orb.GetState() == Orb::State::rest)
    {
      if (length < 16)
      {
        // resting and cable not fully extended, keep orb resting

        return;
      }
      else
      {
        // resting and cable is at full extension, wake orb

        _orb.SetState(this, Orb::State::active);
      }
    }
    
    // constrained length is 8 (x256) units

    int16_t diff = length - 16;

     // player only affected by 1/8 of constraint

    int16_t constrain = diff / 8;

    // normalize to 256 * unit vector

    dir /= length;
  
    // apply constraints

    _player.Move(dir * constrain);
    _orb.Move(-dir * (diff - constrain));
  }
}

void Game::DrawStars()
{
  _stars.Draw(this, _player.GetPosition());
}

void Game::DrawTrainerMenu()
{
  // title

  DrawText(GetDevice(), 64, 1, trainerString, HorizontalAlign::centre, VerticalAlign::top, Color::white);

  // draw the menu

  for (uint8_t item = 0; item < 5; item++)
  {
    int16_t y = 14 + 7 * item;

    if (item == _item)
    {
      GetDevice().fillRect(0, y - 1, 128, 7, WHITE);
    }

    switch (item)
    {
    case 0: DrawText(GetDevice(), 64, y, _settings & Settings::infiniteLives ? infiniteLivesOnString : infiniteLivesOffString, HorizontalAlign::centre, VerticalAlign::top, item == _item ? Color::black : Color::white); break;
    case 1: DrawText(GetDevice(), 64, y, _settings & Settings::infiniteFuel ? infiniteFuelOnString : infiniteFuelOffString, HorizontalAlign::centre, VerticalAlign::top, item == _item ? Color::black : Color::white); break;
    case 2: DrawText(GetDevice(), 64, y, _settings & Settings::gravity ? gravityOnString : gravityOffString, HorizontalAlign::centre, VerticalAlign::top, item == _item ? Color::black : Color::white); break;
    case 3: DrawText(GetDevice(), 64, y, _settings & Settings::collisions ? collisionsOnString : collisionsOffString, HorizontalAlign::centre, VerticalAlign::top, item == _item ? Color::black : Color::white); break;
    case 4: DrawText(GetDevice(), 64, y, _settings & Settings::enemies ? enemiesOnString : enemiesOffString, HorizontalAlign::centre, VerticalAlign::top, item == _item ? Color::black : Color::white); break;
    }
  }
}

void Game::DrawBackground()
{
  _map.Draw(this, _player.GetPosition());

  _refuel.Draw(this, _player.GetPosition());

  _reactor.Draw(this, _player.GetPosition());

  if (GetState() == State::gameExploding)
  {
    GetDevice().invert(_stateTime & 0x1);
  }

  _enemy0.Draw(this, _player.GetPosition());
  _enemy1.Draw(this, _player.GetPosition());
}

void Game::DrawForeground()
{
  bool visible = (_state == State::gameIntro || _state == State::gameOutro) ? (_stateTime & 1) : _state <= State::gameOutro;

  if (visible)
  {
    if (_attached)
    {
      // draw player/orb connection

      int16_t x = _orb.GetPosition().x > _player.GetPosition().x ? (int16_t)((WIDTH / 2) + (_orb.GetPosition().x - _player.GetPosition().x) / 64) + 1 : (int16_t)((WIDTH / 2) + (_orb.GetPosition().x - _player.GetPosition().x) / 64);
      int16_t y = _orb.GetPosition().y > _player.GetPosition().y ? (int16_t)((HEIGHT / 2) + (_orb.GetPosition().y - _player.GetPosition().y) / 64) + 1 : (int16_t)((HEIGHT / 2) + (_orb.GetPosition().y - _player.GetPosition().y) / 64);

      GetDevice().drawLine((WIDTH / 2), (HEIGHT / 2), x, y, WHITE);
    }

    _orb.Draw(this, _player.GetPosition());
    _player.Draw(this);
  }

  _exploder.Draw(this, _player.GetPosition());
}

void Game::DrawHighScores()
{
  DrawStars();

  DrawText(GetDevice(), 64, 1, topThrustersString, HorizontalAlign::centre, VerticalAlign::top, Color::white);

  for (uint8_t entry = 0; entry < 5; entry++)
  {
    int16_t y = 15 + 7 * entry;

    DrawChar(GetDevice(), 24, y, '1' + entry);
    DrawChar(GetDevice(), 24 + 7, y, '.');
    DrawNumber(GetDevice(), 80, y, _scores[entry].Value, HorizontalAlign::right, VerticalAlign::top, Color::white);
    DrawText(GetDevice(), 104, y, _scores[entry].Name, HorizontalAlign::right, VerticalAlign::top, Color::white, false);
  }
}

void Game::DrawPressAToStart()
{
  if (!(_stateTime & 0x40))
  {
    // you know you want to

    GetDevice().fillRect(0, 56, 128, 7, BLACK);
    DrawText(GetDevice(), 64, 57, pressAToStartString, HorizontalAlign::centre, VerticalAlign::top, Color::white);
  }
}

void Game::DrawMissionStart()
{
  // do a little slide

  int16_t offset = 0;
  int16_t half = levelTime / 2;
  if (_stateTime > half)
  {
    int16_t value = _stateTime - half;
    offset = (value * value) >> 2;
  }

  DrawText(GetDevice(), 64 + offset, 32, missionString, HorizontalAlign::centre, VerticalAlign::middle);
  DrawNumber(GetDevice(), 64 - offset, 32 + 6, _level + 1, HorizontalAlign::centre, VerticalAlign::middle);
}

void Game::DrawStats()
{
  // show the player how they're doing

  GetDevice().fillRect(0, 0, 128, 6, BLACK);
  GetDevice().drawLine(0, 6, 128, 6, WHITE);
  DrawNumber(GetDevice(), 0, 0, _fuel, HorizontalAlign::left, VerticalAlign::top);
  DrawNumber(GetDevice(), 64, 0, _lives, HorizontalAlign::centre, VerticalAlign::top);
  DrawNumber(GetDevice(), 128, 0, _displayScore, HorizontalAlign::right, VerticalAlign::top);
}

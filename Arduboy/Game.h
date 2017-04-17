#pragma once

#include "Globals.h"
#include "Common.h"
#include "Stars.h"
#include "Map.h"
#include "Player.h"
#include "Orb.h"
#include "FuelTank.h"
#include "Reactor.h"
#include "Enemy.h"
#include "Particles.h"

class Game
{
private:
  struct Score
  {
    uint16_t Value;
    unsigned char Name[4];
  };

public:
  enum class State : uint8_t
  {
    unspecified,
    splash,
    trainer,
    titleReplay,
    titleScores,
    gameIntro,
    gameActive,
    gameExploding,
    gameOutro,
    gameCompleted,
    gameRetry,
    gameOver,
    newHighScore
  };

  enum Settings : uint8_t
  {
    infiniteLives = 0x01,
    infiniteFuel = 0x02,
    gravity = 0x04,
    collisions = 0x08,
    enemies = 0x10,
  };

  Game() :
    _state(State::unspecified),
    _settings(Settings::gravity | Settings::collisions | Settings::enemies),
    _item(0),
    _enemy0(0),
    _enemy1(1),
    _input(0),
    _replay(0),
    _attached(false)
  {
    GetDevice().setFrameRate(frameRate);
  }

  Arduboy& GetDevice() { return _arduboy; }
  
  void Setup()
  {
    GetDevice().begin();
    InitDebug();
  }
  
  void Loop()
  {
    if (GetDevice().nextFrame())
    {
      Update();
      Render();
      _stateTime++;
    }
  }

  void SetState(const State& state);
  State& GetState() { return _state; }
  
  void Update();
  void Render();

  void Start();
  void InitLevel(uint8_t level);
  void StartLevel();
  void UpdateGame(uint8_t prev, uint8_t input);

  uint8_t GetDoorOffset(uint8_t door)
  {
    uint8_t time = (door * 0x10 + (GetDevice().frameCount >> 2)) & 0x3f;
    switch (time & 0x30)
    {
      case 0x00: return time & 0xf;
      case 0x10:
      case 0x20: return 0xf;
      case 0x30: return 15 - (time & 0xf);
    }

    return 0;
  }
  
  uint8_t GetSettings() { return _settings; }
  Map& GetMap() { return _map; }
  Player& GetPlayer() { return _player; }

private:
  void LoadScores();
  void SaveScores();

  void Constrain();

  void DrawStars();
  void DrawTrainerMenu();
  void DrawBackground();
  void DrawForeground();
  void DrawHighScores();
  void DrawPressAToStart();
  void DrawMissionStart();
  void DrawStats();

private:
  Arduboy _arduboy;

  State _state;
  uint16_t _stateTime;

  uint8_t _settings;

  // re-usable item index for cheat code, menu items and high score entry

  int8_t _item;

  // game objects

  Stars _stars;
  Map _map;
  Player _player;
  Orb _orb;
  Fuel _refuel;
  Reactor _reactor;
  Enemy _enemy0;
  Enemy _enemy1;
  Particles<16> _exploder;
  Score _scores[5];

  // last button inputs

  uint8_t _input;
  uint8_t _replay;

  // player stats

  uint8_t _level;
  uint16_t _fuel;
  uint16_t _score;
  uint16_t _displayScore;
  uint16_t _lives;

  char _name[3];

  // orb attached

  bool _attached;
};


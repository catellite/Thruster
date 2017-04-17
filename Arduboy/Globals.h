#pragma once

#include <avr/pgmspace.h>
#include <stdint.h>

//#define RECORD_GAMEPLAY
#define HAS_RESISTANCE

// game id used to validate eeprom high scores
static const uint16_t gameId = 7000;

// frame rate
static const uint8_t frameRate = 30;

// frames to display splash screen
static const int16_t splashTime = 192;

// frames to display level intro
static const int16_t levelTime = 64;

// frames to display level intro
static const int16_t outroTime = 32;

// frames to display game over
static const int16_t completedTime = 32;

// frames to display game over
static const int16_t gameOverTime = 128;

// constant gravity, applied to velocity.y per frame
static const int16_t gravity = 2;

// air/movement resistance, 1/n is subtracted per frame 
static const int16_t resistance = 128;

// amount to refuel per frame
static const int8_t refuel = 10;

// frames before orb respawn
static const int8_t orbRespawn = 48;

// start at level
static const uint8_t startLevel = 0;

// splash screen
extern const unsigned char splashData[];

// missions
extern const unsigned char missionsData[];


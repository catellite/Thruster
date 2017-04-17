#pragma once

#if defined WIN32
#include "DeviceWin32.h"
#else

#include <avr/power.h>
#include <SPI.h>
#include <avr/sleep.h>
#include <limits.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

// main hardware compile flags

#if !defined(ARDUBOY_10) && !defined(AB_DEVKIT)
/// defaults to Arduboy Release 1.0 if not using a boards.txt file
/**
 * we default to Arduboy Release 1.0 if a compile flag has not been
 * passed to us from a boards.txt file
 *
 * if you wish to compile for the devkit without using a boards.txt
 * file simply comment out the ARDUBOY_10 define and uncomment
 * the AB_DEVKIT define like this:
 *
 *     // #define ARDUBOY_10
 *     #define AB_DEVKIT
 */     
#define ARDUBOY_10   //< compile for the production Arduboy v1.0
// #define AB_DEVKIT    //< compile for the official dev kit
#endif


#ifdef AB_DEVKIT
#define DEVKIT       //< for compatibilty with older sketches
#define SAFE_MODE    //< include safe mode (44 bytes)
#endif 


#ifdef ARDUBOY_10

#define CS 12
#define DC 4
#define RST 6

#define RED_LED 10
#define GREEN_LED 11
#define BLUE_LED 9
#define TX_LED 30
#define RX_LED 17

// pin values for buttons, probably shouldn't use these
#define PIN_LEFT_BUTTON A2
#define PIN_RIGHT_BUTTON A1
#define PIN_UP_BUTTON A0
#define PIN_DOWN_BUTTON A3
#define PIN_A_BUTTON 7
#define PIN_B_BUTTON 8

// bit values for button states
#define LEFT_BUTTON _BV(5)
#define RIGHT_BUTTON _BV(6)
#define UP_BUTTON _BV(7)
#define DOWN_BUTTON _BV(4)
#define A_BUTTON _BV(3)
#define B_BUTTON _BV(2)

#define PIN_SPEAKER_1 5
#define PIN_SPEAKER_2 13

#define PIN_SPEAKER_1_PORT &PORTC
#define PIN_SPEAKER_2_PORT &PORTC

#define PIN_SPEAKER_1_BITMASK _BV(6)
#define PIN_SPEAKER_2_BITMASK _BV(7)

#elif defined(AB_DEVKIT)

#define CS 6
#define DC 4
#define RST 12

// map all LEDs to the single TX LED on DEVKIT
#define RED_LED 17
#define GREEN_LED 17
#define BLUE_LED 17
#define TX_LED 17
#define RX_LED 17

// pin values for buttons, probably shouldn't use these
#define PIN_LEFT_BUTTON 9
#define PIN_RIGHT_BUTTON 5
#define PIN_UP_BUTTON 8
#define PIN_DOWN_BUTTON 10
#define PIN_A_BUTTON A0
#define PIN_B_BUTTON A1

// bit values for button states
#define LEFT_BUTTON _BV(5)
#define RIGHT_BUTTON _BV(2)
#define UP_BUTTON _BV(4)
#define DOWN_BUTTON _BV(6)
#define A_BUTTON _BV(1)
#define B_BUTTON _BV(0)

#define PIN_SPEAKER_1 A2
#define PIN_SPEAKER_2 A3

#define PIN_SPEAKER_1_PORT &PORTF
#define PIN_SPEAKER_2_PORT &PORTF

#define PIN_SPEAKER_1_BITMASK _BV(5)
#define PIN_SPEAKER_2_BITMASK _BV(4)

#endif

// OLED hardware (SSD1306)

#define OLED_PIXELS_INVERTED 0xA7 // All pixels inverted
#define OLED_PIXELS_NORMAL 0xA6 // All pixels normal

#define OLED_ALL_PIXELS_ON 0xA5 // all pixels on
#define OLED_PIXELS_FROM_RAM 0xA4 // pixels mapped to display RAM contents

#define OLED_VERTICAL_FLIPPED 0xC0 // reversed COM scan direction
#define OLED_VERTICAL_NORMAL 0xC8 // normal COM scan direction

// -----

#define COLUMN_ADDRESS_END (WIDTH - 1) & 0x7F   // 128 pixels wide
#define PAGE_ADDRESS_END ((HEIGHT/8)-1) & 0x07  // 8 pages high

#define WIDTH 128
#define HEIGHT 64

#define INVERT 2 //< lit/unlit pixel
#define WHITE 1 //< lit pixel
#define BLACK 0 //< unlit pixel

#define SAFE_MODE

class ArduboyCore
{
public:
    ArduboyCore();

    /// allows the CPU to idle between frames
    /**
     * This puts the CPU in "Idle" sleep mode.  You should call this as often
     * as you can for the best power savings.  The timer 0 overflow interrupt
     * will wake up the chip every 1ms - so even at 60 FPS a well written
     * app should be able to sleep maybe half the time in between rendering
     * it's own frames.
     * 
     * See the Arduboy class nextFrame() for an example of how to use idle()
     * in a frame loop.
     */
    void static idle();

    void static LCDDataMode(); //< put the display in data mode

    /// put the display in command mode
    /**
     * See SSD1306 documents for available commands and command sequences.
     * 
     * Links:
     * - https://www.adafruit.com/datasheets/SSD1306.pdf
     * - http://www.eimodule.com/download/SSD1306-OLED-Controller.pdf
     */
    void static LCDCommandMode();

    uint8_t static width();    //< return display width
    uint8_t static height();   // < return display height

    /// get current state of all buttons (bitmask)
    /**
     * Bit mask that is returned:
     *
     *           Hi   Low   
     *  DevKit   00000000    - reserved                         
     *           -DLU-RAB    D down
     *                       U up       
     *  1.0      00000000    L left
     *           URLDAB--    R right
     * 
     * Of course you shouldn't worry about bits (they may change with future
     * hardware revisions) and should instead use the button defines:
     * LEFT_BUTTON, A_BUTTON, UP_BUTTON, etc.
     */

    uint8_t static getInput(); __attribute__ ((deprecated("use buttonsState() instead")));
    uint8_t static buttonsState();

    // paints 8 pixels (vertically) from a single byte
    //  - 1 is lit, 0 is unlit
    //
    // NOTE: You probably wouldn't actually use this, you'd build something
    // higher level that does it's own calls to SPI.transfer().  It's
    // included for completeness since it seems there should be some very
    // rudimentary low-level draw function in the core that supports the
    // minimum unit that the hardware allows (which is a strip of 8 pixels)
    //
    // This routine starts in the top left and then across the screen.
    // After each "page" (row) of 8 pixels is drawn it will shift down
    // to start drawing the next page.  To paint the full screen you call
    // this function 1,024 times.
    //
    // Example:
    //
    // X = painted pixels, . = unpainted
    //
    // blank()                      paint8Pixels() 0xFF, 0, 0x0F, 0, 0xF0
    // v TOP LEFT corner (8x9)      v TOP LEFT corner
    // ........ (page 1)            X...X... (page 1)
    // ........                     X...X...
    // ........                     X...X...
    // ........                     X...X...
    // ........                     X.X.....
    // ........                     X.X.....
    // ........                     X.X.....
    // ........ (end of page 1)     X.X..... (end of page 1)
    // ........ (page 2)            ........ (page 2)
    void static paint8Pixels(uint8_t pixels);

    /// paints an entire image directly to hardware (from PROGMEM)
    /*
     * Each byte will be 8 vertical pixels, painted in the same order as
     * explained above in paint8Pixels.
     */
    void static paintScreen(const unsigned char *image);

    /// paints an entire image directly to hardware (from RAM)
    /*
     * Each byte will be 8 vertical pixels, painted in the same order as
     * explained above in paint8Pixels.
     */
    void static paintScreen(unsigned char image[]);

    /// paints a blank (black) screen to hardware
    void static blank();

    /// invert the display or set to normal
    /**
     * when inverted, a pixel set to 0 will be on
     */
    void static invert(boolean inverse);

    /// turn all display pixels on, or display the buffer contents
    /**
     * when set to all pixels on, the display buffer will be
     * ignored but not altered
     */
    void static allPixelsOn(boolean on);

    /// flip the display vertically or set to normal
    void static flipVertical(boolean flipped);

    /// flip the display horizontally or set to normal
    void static flipHorizontal(boolean flipped);

    /// send a single byte command to the OLED
    void static sendLCDCommand(uint8_t command);

    /// set the light output of the RGB LEB
    void setRGBled(uint8_t red, uint8_t green, uint8_t blue);

protected:
    /// boots the hardware
    /**
     * - sets input/output/pullup mode for pins
     * - powers up the OLED screen and initializes it properly
     * - sets up power saving
     * - kicks CPU down to 8Mhz if needed
     * - allows Safe mode to be entered
     */
    void static boot();

    /// Safe mode
    /**
     * Safe Mode is engaged by holding down both the LEFT button and UP button
     * when plugging the device into USB.  It puts your device into a tight
     * loop and allows it to be reprogrammed even if you have uploaded a very
     * broken sketch that interferes with the normal USB triggered auto-reboot
     * functionality of the device.
     * 
     * This is most useful on Devkits because they lack a built-in reset
     * button.
     */
    void static safeMode() __attribute__((always_inline));

    // internals
    void static bootLCD() __attribute__((always_inline));
    void static bootPins() __attribute__((always_inline));
    void static slowCPU() __attribute__((always_inline));
    void static saveMuchPower(); __attribute__((always_inline));

private:
    volatile static uint8_t *mosiport, *csport, *dcport;
    uint8_t static mosipinmask, cspinmask, dcpinmask;
};

/// Library version.
/**
 * A version number, 'x.y.z', is stored in the form xxyyzz,
 * where ((x * 10000) + (y * 100) + (z)),
 * resulting in 'xxxyyzz', with no leading zeros.
 */
#define ARDUBOY_LIB_VER 10101 // 1.1.1

// EEPROM settings
#define EEPROM_VERSION 0
#define EEPROM_BRIGHTNESS 1
#define EEPROM_AUDIO_ON_OFF 2

// we reserve the first 16 byte of EEPROM for system use
#define EEPROM_STORAGE_SPACE_START 16 // and onward

// eeprom settings above are neded for audio
#define AVAILABLE_TIMERS 2
#define TUNE_OP_PLAYNOTE  0x90  /* play a note: low nibble is generator #, note is next byte */
#define TUNE_OP_STOPNOTE  0x80  /* stop a note: low nibble is generator # */
#define TUNE_OP_RESTART   0xe0  /* restart the score from the beginning */
#define TUNE_OP_STOP      0xf0  /* stop playing */



#define PIXEL_SAFE_MODE

// compare Vcc to 1.1 bandgap
#define ADC_VOLTAGE (_BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
// compare temperature to 2.5 internal reference and _BV(MUX5)
#define ADC_TEMP (_BV(REFS0) | _BV(REFS1) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0))

class Arduboy : public ArduboyCore
{
public:
  Arduboy();

  /// Returns true if the button mask passed in is pressed.
  /**
   * if (pressed(LEFT_BUTTON + A_BUTTON))
   */
  boolean pressed(uint8_t buttons);

  /// Returns true if the button mask passed in not pressed.
  /**
   * if (notPressed(LEFT_BUTTON))
   */
  boolean notPressed(uint8_t buttons);

  /// Initializes the hardware
  void begin();

  /// Clears display.
  void clear();
  void clearDisplay() __attribute__ ((deprecated("use clear() instead")));

  /// Copies the contents of the screen buffer to the screen.
  /**
   * X and Y positions on the display are from the top left corner, thus a Y of 64
   * is the bottom of the screen and an X of 128 is the right side of the screen.
   * "Color" or "value" means choosing whether a pixel is lit or not - if color is
   * 0, the pixel is off (black), if color is 1, the pixel is on (white).
   */
  void display();

  /// Sets a single pixel on the screen buffer to white or black.
  void drawPixel(int x, int y, uint8_t color);

  uint8_t getPixel(uint8_t x, uint8_t y);

  /// Draw a circle of a defined radius.
  /**
   * Draws a circle in white or black. X and Y are the center point of the circle.
   */
  void drawCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color);

  /// Draws one or more "corners" of a circle.
  void drawCircleHelper(int16_t x0, int16_t y0, uint8_t r, uint8_t cornername, uint8_t color);

  /// Draws a filled-in circle.
  void fillCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color);

   /// Draws one or both vertical halves of a filled-in circle.
  void fillCircleHelper(int16_t x0, int16_t y0, uint8_t r, uint8_t cornername, int16_t delta, uint8_t color);

  /// Draws a line between two points.
  /**
   * Uses Bresenham's algorithm.
   */
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color);

  /// Draws a rectangle of a width and height.
  void drawRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color);

  /// Draws vertical line.
  void drawFastVLine(int16_t x, int16_t y, uint8_t h, uint8_t color);

  /// Draws a horizontal line.
  void drawFastHLine(int16_t x, int16_t y, uint8_t w, uint8_t color);

  /// Draws a filled-in rectangle.
  void fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color);

  /// Fills the screen buffer with white or black.
  void fillScreen(uint8_t color);

  /// Draws a rectangle with rounded edges.
  void drawRoundRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color);

  /// Draws a filled-in rectangle with rounded edges.
  void fillRoundRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color);

   /// Draws the outline of a triangle.
  void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color);

  /// Draws a filled-in triangle.
  void fillTriangle (int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color);

  /// Draws a bitmap from program memory to a specific X/Y
  void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color);

  /// Draws images that are bit-oriented horizontally.
  /**
   * This requires a lot of additional CPU power and will draw images slower
   * than drawBitmap, where the images are stored in a format that
   * allows them to be directly written to the screen. It is
   * recommended you use drawBitmap when possible.
   */
  void drawSlowXYBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color);

  /// Set text size
  /**
   * As mentioned in drawChar(), individual ASCII characters are 6x8 pixels
   * (5x7 with spacing on two edges). The size is a pixel multiplier,
   * so a size of 2 means each character will be 12x16, etc.
   */
  void setTextSize(uint8_t s);

  /// Sets whether text will wrap at screen edges.
  void setTextWrap(boolean w);

  unsigned char* getBuffer();

  /// Writes a single ASCII character to the screen.
  virtual size_t write(uint8_t) { }
  
  /// Seeds the random number generator with entropy from the temperature, voltage reading, and microseconds since boot.
  /**
   * This method is still most effective when called semi-randomly such
   * as after a user hits a button to start a game or other semi-random
   * events
   */
  void initRandomSeed();

  /// Swap the references of two pointers.
  void swap(int16_t& a, int16_t& b);

  //ArduboyAudio Audio;

  void setFrameRate(uint8_t rate);
  bool nextFrame();
  bool everyXFrames(uint8_t frames);
  
  /// Returns the load on the CPU as a percentage.
  /**
   * This is based on how much of the time your app is spends rendering
   * frames.  This number can be higher than 100 if your app is rendering
   * really slowly.
   */
  int cpuLoad();
  
  uint8_t frameRate;
  uint16_t frameCount;
  uint8_t eachFrameMillis;
  long lastFrameStart;
  long nextFrameStart;
  bool post_render;
  uint8_t lastFrameDurationMs;

  /// useful for getting raw approximate voltage values
  uint16_t rawADC(byte adc_bits);

protected:
  unsigned char sBuffer[(HEIGHT*WIDTH)/8];
};

#endif

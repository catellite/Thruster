#include "Device.h"

#if !defined WIN32

#include <avr/io.h>
#include <avr/pgmspace.h>

// need to redeclare these here since we declare them static in .h
volatile uint8_t *ArduboyCore::mosiport, *ArduboyCore::csport, *ArduboyCore::dcport;
uint8_t ArduboyCore::mosipinmask, ArduboyCore::cspinmask, ArduboyCore::dcpinmask;

const uint8_t PROGMEM pinBootProgram[] =
{
  // buttons
  PIN_LEFT_BUTTON, INPUT_PULLUP,
  PIN_RIGHT_BUTTON, INPUT_PULLUP,
  PIN_UP_BUTTON, INPUT_PULLUP,
  PIN_DOWN_BUTTON, INPUT_PULLUP,
  PIN_A_BUTTON, INPUT_PULLUP,
  PIN_B_BUTTON, INPUT_PULLUP,

  // OLED SPI
  DC, OUTPUT,
  CS, OUTPUT,
  RST, OUTPUT,
  0
};

const uint8_t PROGMEM lcdBootProgram[] = {
  // boot defaults are commented out but left here incase they
  // might prove useful for reference
  //
  // Further reading: https://www.adafruit.com/datasheets/SSD1306.pdf
  //
  // Display Off
  // 0xAE,     

  // Set Display Clock Divisor v = 0xF0
  // default is 0x80
  0xD5, 0xF0,

  // Set Multiplex Ratio v = 0x3F
  // 0xA8, 0x3F,   

  // Set Display Offset v = 0
  // 0xD3, 0x00, 

  // Set Start Line (0)
  // 0x40,     

  // Charge Pump Setting v = enable (0x14)
  // default is disabled
  0x8D, 0x14,   

  // Set Segment Re-map (A0) | (b0001)
  // default is (b0000)
  0xA1,     

  // Set COM Output Scan Direction
  0xC8,     

  // Set COM Pins v
  // 0xDA, 0x12,   

  // Set Contrast v = 0xCF
  0x81, 0xCF,   

  // Set Precharge = 0xF1
  0xD9, 0xF1,
    
  // Set VCom Detect
  // 0xDB, 0x40,   

  // Entire Display ON
  // 0xA4,     

  // Set normal/inverse display
  // 0xA6,  

  // Display On
  0xAF,     

  // set display mode = horizontal addressing mode (0x00)
  0x20, 0x00,  

  // set col address range 
  // 0x21, 0x00, COLUMN_ADDRESS_END,

  // set page address range
  // 0x22, 0x00, PAGE_ADDRESS_END
};

ArduboyCore::ArduboyCore() {}

void ArduboyCore::boot()
{
  #if F_CPU == 8000000L
  slowCPU();
  #endif

  SPI.begin();
  bootPins();
  bootLCD();

#ifdef SAFE_MODE
  if (buttonsState() == (LEFT_BUTTON | UP_BUTTON))
    safeMode();
#endif

  saveMuchPower();
}

#if F_CPU == 8000000L
// if we're compiling for 8Mhz we need to slow the CPU down because the
// hardware clock on the Arduboy is 16MHz
void ArduboyCore::slowCPU()
{
  uint8_t oldSREG = SREG;
  cli();                // suspend interrupts
  CLKPR = _BV(CLKPCE);  // allow reprogramming clock
  CLKPR = 1;            // set clock divisor to 2 (0b0001)
  SREG = oldSREG;       // restore interrupts
}
#endif

void ArduboyCore::bootPins()
{
  uint8_t pin, mode;
  const uint8_t *i = pinBootProgram;

  while(true) {
    pin = pgm_read_byte(i++);
    mode = pgm_read_byte(i++);
    if (pin==0) break;
    pinMode(pin, mode);
  }

  digitalWrite(RST, HIGH);
  delay(1);           // VDD (3.3V) goes high at start, lets just chill for a ms
  digitalWrite(RST, LOW);   // bring reset low
  delay(10);          // wait 10ms
  digitalWrite(RST, HIGH);  // bring out of reset
}

void ArduboyCore::bootLCD()
{
  // setup the ports we need to talk to the OLED
  csport = portOutputRegister(digitalPinToPort(CS));
  cspinmask = digitalPinToBitMask(CS);
  dcport = portOutputRegister(digitalPinToPort(DC));
  dcpinmask = digitalPinToBitMask(DC);

  SPI.setClockDivider(SPI_CLOCK_DIV2);

  LCDCommandMode();
  // run our customized boot-up command sequence against the
  // OLED to initialize it properly for Arduboy
  for (int8_t i=0; i < sizeof(lcdBootProgram); i++) {
    SPI.transfer(pgm_read_byte(lcdBootProgram + i));
  }
  LCDDataMode();
}

void ArduboyCore::LCDDataMode()
{
  *dcport |= dcpinmask;
  *csport &= ~cspinmask;
}

void ArduboyCore::LCDCommandMode()
{
  *csport |= cspinmask;
  *dcport &= ~dcpinmask;
  *csport &= ~cspinmask;
}

void ArduboyCore::safeMode()
{
  blank(); // too avoid random gibberish
  while (true)
  {
    asm volatile("nop \n");
  }
}

/* Power Management */

void ArduboyCore::idle()
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_mode();
}

void ArduboyCore::saveMuchPower()
{
  power_adc_disable();
  power_usart0_disable();
  power_twi_disable();
  // timer 0 is for millis()
  // timers 1 and 3 are for music and sounds
  power_timer2_disable();
  power_usart1_disable();
  // we need USB, for now (to allow triggered reboots to reprogram)
  // power_usb_disable()
}

uint8_t ArduboyCore::width() { return WIDTH; }

uint8_t ArduboyCore::height() { return HEIGHT; }

/* Drawing */

void ArduboyCore::paint8Pixels(uint8_t pixels)
{
  SPI.transfer(pixels);
}

void ArduboyCore::paintScreen(const unsigned char *image)
{
  for (int i = 0; i < (HEIGHT*WIDTH)/8; i++)
  {
    SPI.transfer(pgm_read_byte(image + i));
  }
}

// paint from a memory buffer, this should be FAST as it's likely what
// will be used by any buffer based subclass
void ArduboyCore::paintScreen(unsigned char image[])
{
  uint8_t c;
  int i = 0;

  SPDR = image[i++]; // set the first SPI data byte to get things started

  // the code to iterate the loop and get the next byte from the buffer is
  // executed while the previous byte is being sent out by the SPI controller
  while (i < (HEIGHT * WIDTH) / 8)
  {
    // get the next byte. It's put in a local variable so it can be sent as
    // as soon as possible after the sending of the previous byte has completed
    c = image[i++];

    while (!(SPSR & _BV(SPIF))) { } // wait for the previous byte to be sent

    // put the next byte in the SPI data register. The SPI controller will
    // clock it out while the loop continues and gets the next byte ready
    SPDR = c;
  }
  while (!(SPSR & _BV(SPIF))) { } // wait for the last byte to be sent
}

void ArduboyCore::blank()
{
  for (int i = 0; i < (HEIGHT*WIDTH)/8; i++)
    SPI.transfer(0x00);
}

void ArduboyCore::sendLCDCommand(uint8_t command)
{
  LCDCommandMode();
  SPI.transfer(command);
  LCDDataMode();
}

// invert the display or set to normal
// when inverted, a pixel set to 0 will be on
void ArduboyCore::invert(boolean inverse)
{
  sendLCDCommand(inverse ? OLED_PIXELS_INVERTED : OLED_PIXELS_NORMAL);
}

// turn all display pixels on, ignoring buffer contents
// or set to normal buffer display
void ArduboyCore::allPixelsOn(boolean on)
{
  sendLCDCommand(on ? OLED_ALL_PIXELS_ON : OLED_PIXELS_FROM_RAM);
}

// flip the display vertically or set to normal
void ArduboyCore::flipVertical(boolean flipped)
{
  sendLCDCommand(flipped ? OLED_VERTICAL_FLIPPED : OLED_VERTICAL_NORMAL);
}

#define OLED_HORIZ_FLIPPED 0xA0 // reversed segment re-map
#define OLED_HORIZ_NORMAL 0xA1 // normal segment re-map

// flip the display horizontally or set to normal
void ArduboyCore::flipHorizontal(boolean flipped)
{
  sendLCDCommand(flipped ? OLED_HORIZ_FLIPPED : OLED_HORIZ_NORMAL);
}

/* RGB LED */

void ArduboyCore::setRGBled(uint8_t red, uint8_t green, uint8_t blue)
{
#ifdef ARDUBOY_10 // RGB, all the pretty colors
  // inversion is necessary because these are common annode LEDs
  analogWrite(RED_LED, 255 - red);
  analogWrite(GREEN_LED, 255 - green);
  analogWrite(BLUE_LED, 255 - blue);
#elif defined(AB_DEVKIT)
  // only blue on devkit
  digitalWrite(BLUE_LED, ~blue);
#endif
}

/* Buttons */

uint8_t ArduboyCore::getInput()
{
  return buttonsState();
}

uint8_t ArduboyCore::buttonsState()
{
  uint8_t buttons;
  
  // using ports here is ~100 bytes smaller than digitalRead()
#ifdef AB_DEVKIT
  // down, left, up
  buttons = ((~PINB) & B01110000);
  // right button
  buttons = buttons | (((~PINC) & B01000000) >> 4);
  // A and B
  buttons = buttons | (((~PINF) & B11000000) >> 6);
#elif defined(ARDUBOY_10)
  // down, up, left right
  buttons = ((~PINF) & B11110000);
  // A (left)
  buttons = buttons | (((~PINE) & B01000000) >> 3);
  // B (right)
  buttons = buttons | (((~PINB) & B00010000) >> 2);
#endif
  
  return buttons;
}

Arduboy::Arduboy()
{
  // frame management
  setFrameRate(60);
  frameCount = 0;
  nextFrameStart = 0;
  post_render = false;
  // init not necessary, will be reset after first use
  // lastFrameStart
  // lastFrameDurationMs
}

void Arduboy::begin()
{
  boot(); // required
}

/* Frame management */

void Arduboy::setFrameRate(uint8_t rate)
{
  frameRate = rate;
  eachFrameMillis = 1000/rate;
}

bool Arduboy::everyXFrames(uint8_t frames)
{
  return frameCount % frames == 0;
}

bool Arduboy::nextFrame()
{
  long now = millis();
  uint8_t remaining;

  // post render
  if (post_render) {
    lastFrameDurationMs = now - lastFrameStart;
    frameCount++;
    post_render = false;
  }

  // if it's not time for the next frame yet
  if (now < nextFrameStart) {
    remaining = nextFrameStart - now;
    // if we have more than 1ms to spare, lets sleep
    // we should be woken up by timer0 every 1ms, so this should be ok
    if (remaining > 1)
      idle();
    return false;
  }

  // pre-render

  // technically next frame should be last frame + each frame but if we're
  // running a slow render we would constnatly be behind the clock
  // keep an eye on this and see how it works.  If it works well the
  // lastFrameStart variable could be eliminated completely
  nextFrameStart = now + eachFrameMillis;
  lastFrameStart = now;
  post_render = true;
  return post_render;
}

int Arduboy::cpuLoad()
{
  return lastFrameDurationMs*100 / eachFrameMillis;
}

void Arduboy::initRandomSeed()
{
  power_adc_enable(); // ADC on
  randomSeed(~rawADC(ADC_TEMP) * ~rawADC(ADC_VOLTAGE) * ~micros() + micros());
  power_adc_disable(); // ADC off
}

uint16_t Arduboy::rawADC(byte adc_bits)
{
  ADMUX = adc_bits;
  // we also need MUX5 for temperature check
  if (adc_bits == ADC_TEMP) {
    ADCSRB = _BV(MUX5);
  }

  delay(2); // Wait for ADMUX setting to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  return ADC;
}

/* Graphics */

void Arduboy::clearDisplay() // deprecated
{
  clear();
}

void Arduboy::clear()
{
  fillScreen(BLACK);
}

void Arduboy::drawPixel(int x, int y, uint8_t color)
{
  #ifdef PIXEL_SAFE_MODE
  if (x < 0 || x > (WIDTH-1) || y < 0 || y > (HEIGHT-1))
  {
    return;
  }
  #endif

  uint8_t row = (uint8_t)y / 8;
  if (color)
  {
    sBuffer[(row*WIDTH) + (uint8_t)x] |=   _BV((uint8_t)y & 0x7);
  }
  else
  {
    sBuffer[(row*WIDTH) + (uint8_t)x] &= ~ _BV((uint8_t)y & 0x7);
  }
}

uint8_t Arduboy::getPixel(uint8_t x, uint8_t y)
{
  uint8_t row = y / 8;
  uint8_t bit_position = y & 0x7;
  return (sBuffer[(row*WIDTH) + x] & _BV(bit_position)) >> bit_position;
}

void Arduboy::drawCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color)
{
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  drawPixel(x0, y0+r, color);
  drawPixel(x0, y0-r, color);
  drawPixel(x0+r, y0, color);
  drawPixel(x0-r, y0, color);

  while (x<y)
  {
    if (f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }
}

void Arduboy::drawCircleHelper
(int16_t x0, int16_t y0, uint8_t r, uint8_t cornername, uint8_t color)
{
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x<y)
  {
    if (f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    if (cornername & 0x4)
    {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    }
    if (cornername & 0x2)
    {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8)
    {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1)
    {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

void Arduboy::fillCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color)
{
  drawFastVLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

void Arduboy::fillCircleHelper
(int16_t x0, int16_t y0, uint8_t r, uint8_t cornername, int16_t delta,
 uint8_t color)
{
  // used to do circles and roundrects!
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x < y)
  {
    if (f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    if (cornername & 0x1)
    {
      drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
    }

    if (cornername & 0x2)
    {
      drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}

void Arduboy::drawLine
(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
  // bresenham's algorithm - thx wikpedia
  boolean steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int8_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  }
  else
  {
    ystep = -1;
  }

  for (; x0 <= x1; x0++)
  {
    if (steep)
    {
      drawPixel(y0, x0, color);
    }
    else
    {
      drawPixel(x0, y0, color);
    }

    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void Arduboy::drawRect
(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color)
{
  drawFastHLine(x, y, w, color);
  drawFastHLine(x, y+h-1, w, color);
  drawFastVLine(x, y, h, color);
  drawFastVLine(x+w-1, y, h, color);
}

void Arduboy::drawFastVLine
(int16_t x, int16_t y, uint8_t h, uint8_t color)
{
  int end = y+h;
  for (int a = max(0,y); a < min(end,HEIGHT); a++)
  {
    drawPixel(x,a,color);
  }
}

void Arduboy::drawFastHLine
(int16_t x, int16_t y, uint8_t w, uint8_t color)
{
  int end = x+w;
  for (int a = max(0,x); a < min(end,WIDTH); a++)
  {
    drawPixel(a,y,color);
  }
}

void Arduboy::fillRect
(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color)
{
  // stupidest version - update in subclasses if desired!
  for (int16_t i=x; i<x+w; i++)
  {
    drawFastVLine(i, y, h, color);
  }
}

void Arduboy::fillScreen(uint8_t color)
{
  // C version : 
  //
  // if (color) color = 0xFF;  //change any nonzero argument to b11111111 and insert into screen array.
  // for(int16_t i=0; i<1024; i++)  { sBuffer[i] = color; }  //sBuffer = (128*64) = 8192/8 = 1024 bytes. 
  
  asm volatile
  (
    // load color value into r27
    "mov r27, %1 \n\t"
    // if value is zero, skip assigning to 0xff
    "cpse r27, __zero_reg__ \n\t"
    "ldi r27, 0xff \n\t"
    // load sBuffer pointer into Z
    "movw  r30, %0\n\t"
    // counter = 0
    "clr __tmp_reg__ \n\t"
    "loopto:   \n\t"
    // (4x) push zero into screen buffer,
    // then increment buffer position
    "st Z+, r27 \n\t"
    "st Z+, r27 \n\t"
    "st Z+, r27 \n\t"
    "st Z+, r27 \n\t"
    // increase counter
    "inc __tmp_reg__ \n\t"
    // repeat for 256 loops
    // (until counter rolls over back to 0)
    "brne loopto \n\t"
    // input: sBuffer, color
    // modified: Z (r30, r31), r27
    :
    : "r" (sBuffer), "r" (color)
    : "r30", "r31", "r27"
  );
}

void Arduboy::drawRoundRect
(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color)
{
  // smarter version
  drawFastHLine(x+r, y, w-2*r, color); // Top
  drawFastHLine(x+r, y+h-1, w-2*r, color); // Bottom
  drawFastVLine(x, y+r, h-2*r, color); // Left
  drawFastVLine(x+w-1, y+r, h-2*r, color); // Right
  // draw four corners
  drawCircleHelper(x+r, y+r, r, 1, color);
  drawCircleHelper(x+w-r-1, y+r, r, 2, color);
  drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
  drawCircleHelper(x+r, y+h-r-1, r, 8, color);
}

void Arduboy::fillRoundRect
(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color)
{
  // smarter version
  fillRect(x+r, y, w-2*r, h, color);

  // draw four corners
  fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  fillCircleHelper(x+r, y+r, r, 2, h-2*r-1, color);
}

void Arduboy::drawTriangle
(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

void Arduboy::fillTriangle
(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color)
{

  int16_t a, b, y, last;
  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1)
  {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2)
  {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1)
  {
    swap(y0, y1); swap(x0, x1);
  }

  if(y0 == y2)
  { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)
    {
      a = x1;
    }
    else if(x1 > b)
    {
      b = x1;
    }
    if(x2 < a)
    {
      a = x2;
    }
    else if(x2 > b)
    {
      b = x2;
    }
    drawFastHLine(a, y0, b-a+1, color);
    return;
  }

  int16_t dx01 = x1 - x0,
      dy01 = y1 - y0,
      dx02 = x2 - x0,
      dy02 = y2 - y0,
      dx12 = x2 - x1,
      dy12 = y2 - y1,
      sa = 0,
      sb = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if (y1 == y2)
  {
    last = y1;   // Include y1 scanline
  }
  else
  {
    last = y1-1; // Skip it
  }


  for(y = y0; y <= last; y++)
  {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;

    if(a > b)
    {
      swap(a,b);
    }

    drawFastHLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);

  for(; y <= y2; y++)
  {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;

    if(a > b)
    {
      swap(a,b);
    }

    drawFastHLine(a, y, b-a+1, color);
  }
}

void Arduboy::drawBitmap
(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, 
 uint8_t color)
{
  // no need to dar at all of we're offscreen
  if (x+w < 0 || x > WIDTH-1 || y+h < 0 || y > HEIGHT-1)
    return;

  int yOffset = abs(y) & 0x7;
  int sRow = y / 8;
  if (y < 0) {
    sRow--;
    yOffset = 8 - yOffset;
  }
  int rows = h/8;
  if (h & 0x7 != 0) rows++;
  for (int a = 0; a < rows; a++) {
    int bRow = sRow + a;
    if (bRow > (HEIGHT/8)-1) break;
    if (bRow > -2) {
      for (int iCol = 0; iCol<w; iCol++) {
        if (iCol + x > (WIDTH-1)) break;
        if (iCol + x >= 0) {
          if (bRow >= 0) {
            if      (color == WHITE) this->sBuffer[ (bRow*WIDTH) + x + iCol ] |= pgm_read_byte(bitmap+(a*w)+iCol) << yOffset;
            else if (color == BLACK) this->sBuffer[ (bRow*WIDTH) + x + iCol ] &= ~(pgm_read_byte(bitmap+(a*w)+iCol) << yOffset);
            else                     this->sBuffer[ (bRow*WIDTH) + x + iCol ] ^= pgm_read_byte(bitmap+(a*w)+iCol) << yOffset;
          }
          if (yOffset && bRow<(HEIGHT/8)-1 && bRow > -2) {
            if      (color == WHITE) this->sBuffer[ ((bRow+1)*WIDTH) + x + iCol ] |= pgm_read_byte(bitmap+(a*w)+iCol) >> (8-yOffset);
            else if (color == BLACK) this->sBuffer[ ((bRow+1)*WIDTH) + x + iCol ] &= ~(pgm_read_byte(bitmap+(a*w)+iCol) >> (8-yOffset));
            else                     this->sBuffer[ ((bRow+1)*WIDTH) + x + iCol ] ^= pgm_read_byte(bitmap+(a*w)+iCol) >> (8-yOffset);
          }
        }
      }
    }
  }
}

void Arduboy::display()
{
  this->paintScreen(sBuffer);
}

unsigned char* Arduboy::getBuffer()
{
  return sBuffer;
}


boolean Arduboy::pressed(uint8_t buttons)
{
  return (buttonsState() & buttons) == buttons;
}

boolean Arduboy::notPressed(uint8_t buttons)
{
  return (buttonsState() & buttons) == 0;
}

void Arduboy::swap(int16_t& a, int16_t& b)
{
  int temp = a;
  a = b;
  b = temp;
}

#endif

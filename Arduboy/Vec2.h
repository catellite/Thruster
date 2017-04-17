#pragma once 

#include <math.h> 
#include <stdint.h>

template <typename T>
struct vec2
{
  T x, y;
  
  vec2<T>() :
	x(0),
	y(0) { }

  vec2<T>(const T x0, const T y0) :
    x(x0),
	y(y0) { }

  T& operator [](unsigned int i) { return *(&x + i); }
  const T& operator [](unsigned int i) const { return *(&x + i); }

  void operator ()(const T x0, const T y0) { x = x0; y = y0; }

  bool operator ==(const vec2<T>& v) { return (x == v.x && y == v.y); }
  bool operator !=(const vec2<T>& v) { return (x != v.x || y != v.y); }
  
  const vec2<T>& operator =(const vec2<T> &v)
  { 
    x = v.x; y = v.y;
    return *this;
  }
  
  const vec2<T> operator -(void) const { return vec2<T>(-x, -y); }
  
  const vec2<T> operator +(const vec2<T>& v) const { return vec2<T>(x + v.x, y + v.y); }
  const vec2<T> operator -(const vec2<T>& v) const { return vec2<T>(x - v.x, y - v.y); }
    
  const vec2<T> operator *(const T num) const
  {
    vec2<T> temp(*this);     
    return temp *= num;
  }

  const vec2<T> operator /(const T num) const
  {
    vec2<T> temp(*this);     
    return temp /= num;
  }   

  const vec2<T> operator +(const T& v) const { return vec2<T>(x + v, y + v); }

  const vec2<T>& operator +=(const vec2<T>& v)
  {
    x += v.x; y += v.y;           
    return *this;
  }

  const vec2<T>& operator -=(const vec2<T>& v)
  { 
    x -= v.x; y -= v.y;           
    return *this; 
  }

  const vec2<T>& operator *=(const T num)
  {
    x *= num; y *= num;                   
    return *this;
  }
  
  const vec2<T>& operator /=(const T num)
  {
    x /= num; y /= num;                   
    return *this;
  }

  T operator *(const vec2<T> &v) const { return x * v.x + y * v.y; }
};

struct vec2u8: public vec2<uint8_t>
{
  vec2u8() { }
  vec2u8(const vec2<uint8_t>& v): vec2<uint8_t>(v.x, v.y) { }
  vec2u8(uint8_t x0, uint8_t y0): vec2<uint8_t>(x0, y0) { }
};

struct vec2s16: public vec2<int16_t>
{
  vec2s16() { }
  vec2s16(const vec2<int16_t>& v): vec2<int16_t>(v.x, v.y) { }
  vec2s16(int16_t x0, int16_t y0): vec2<int16_t>(x0, y0) { }
};

struct vec2s8: public vec2<int8_t>
{
  vec2s8() { }
  vec2s8(const vec2<int8_t>& v): vec2<int8_t>(v.x, v.y) { }
  vec2s8(int8_t x0, int8_t y0): vec2<int8_t>(x0, y0) { }
  
  operator vec2s16() { return vec2<int16_t>(x, y); }
};


#pragma once
// 공용 기본 타입: 벡터, 사각형, 색상, 수학 유틸.
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace gengar {

constexpr float kPi = 3.14159265358979f;

struct Vec2 {
  float x = 0.f, y = 0.f;
  constexpr Vec2() = default;
  constexpr Vec2(float x_, float y_) : x(x_), y(y_) {}
  constexpr Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
  constexpr Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
  constexpr Vec2 operator*(float s) const { return {x * s, y * s}; }
  Vec2& operator+=(Vec2 o) { x += o.x; y += o.y; return *this; }
  float Length() const { return std::sqrt(x * x + y * y); }
};

struct Rect {
  float x = 0.f, y = 0.f, w = 0.f, h = 0.f;
  float Right() const { return x + w; }
  float Bottom() const { return y + h; }
  Vec2 Center() const { return {x + w * 0.5f, y + h * 0.5f}; }
  bool Overlaps(const Rect& o) const {
    return x < o.x + o.w && x + w > o.x && y < o.y + o.h && y + h > o.y;
  }
};

struct Color {
  uint8_t r = 255, g = 255, b = 255, a = 255;
};

constexpr Color Rgb(int r, int g, int b, int a = 255) {
  return {static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), static_cast<uint8_t>(a)};
}
constexpr Color Hex(uint32_t rgb, int a = 255) {
  return Rgb(static_cast<int>((rgb >> 16) & 255), static_cast<int>((rgb >> 8) & 255), static_cast<int>(rgb & 255), a);
}
inline Color WithAlpha(Color c, float alpha01) {
  c.a = static_cast<uint8_t>(std::clamp(c.a * alpha01, 0.f, 255.f));
  return c;
}
inline Color Lerp(Color a, Color b, float t) {
  auto m = [t](int x, int y) { return static_cast<int>(x + (y - x) * t); };
  return Rgb(m(a.r, b.r), m(a.g, b.g), m(a.b, b.b), m(a.a, b.a));
}
// h: 0~360, s/l: 0~1
inline Color Hsl(float h, float s, float l, float a = 1.f) {
  h = std::fmod(h, 360.f);
  if (h < 0) h += 360.f;
  const float c = (1.f - std::fabs(2.f * l - 1.f)) * s;
  const float x = c * (1.f - std::fabs(std::fmod(h / 60.f, 2.f) - 1.f));
  const float m = l - c / 2.f;
  float r = 0, g = 0, b = 0;
  if (h < 60) { r = c; g = x; }
  else if (h < 120) { r = x; g = c; }
  else if (h < 180) { g = c; b = x; }
  else if (h < 240) { g = x; b = c; }
  else if (h < 300) { r = x; b = c; }
  else { r = c; b = x; }
  return Rgb(static_cast<int>((r + m) * 255), static_cast<int>((g + m) * 255), static_cast<int>((b + m) * 255),
             static_cast<int>(a * 255));
}

template <class T>
constexpr T Clamp(T v, T lo, T hi) { return std::max(lo, std::min(hi, v)); }
inline int Sign(float v) { return (v > 0.f) - (v < 0.f); }

}  // namespace gengar

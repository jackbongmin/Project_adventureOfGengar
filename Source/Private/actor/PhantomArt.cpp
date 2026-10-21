#include "actor/PhantomArt.h"

#include <cmath>

#include "art/Art.h"
#include "platform/Graphics.h"

namespace gengar {

void DrawPhantom(Graphics& g, Vec2 center, int facing, float t, float scale, float alpha) {
  const art::Assets& a = art::A();
  const int ps = std::max(1, static_cast<int>(std::lround(static_cast<float>(art::kPixel) * scale)));  // 도트 1개의 화면 크기
  const SpriteId id = a.phantom[(static_cast<int>(t / 10.f)) % 2];
  const Vec2 sz = g.SpriteSize(id);
  const float bob = std::round(std::sin(t * 0.08f) * 0.9f) * static_cast<float>(ps);  // 도트 단위로 둥실
  g.DrawSprite(id, center.x - sz.x * static_cast<float>(ps) * 0.5f, center.y - sz.y * static_cast<float>(ps) * 0.5f + bob, ps,
               facing < 0, alpha);
}

void DrawShadowBall(Graphics& g, Vec2 c, float r, float t) {
  if (r < 3.f) return;
  const art::Assets& a = art::A();
  const int ps = Clamp(static_cast<int>(std::lround(r * 2.f / 8.f)), 1, 4);
  const SpriteId id = a.ball[(static_cast<int>(t / 4.f)) % 2];
  g.DrawSprite(id, c.x - 4.f * static_cast<float>(ps), c.y - 4.f * static_cast<float>(ps), ps);
}

}  // namespace gengar

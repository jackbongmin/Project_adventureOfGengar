#include "actor/PhantomArt.h"

#include <cmath>

#include "art/Art.h"
#include "platform/Graphics.h"

namespace gengar {
namespace {

int PixelSize(float scale) { return std::max(1, static_cast<int>(std::lround(static_cast<float>(art::kPixel) * scale))); }  // 도트 1개의 화면 크기
float Bob(float t, int ps) { return std::round(std::sin(t * 0.08f) * 0.9f) * static_cast<float>(ps); }  // 도트 단위로 둥실

}  // namespace

void DrawPhantom(Graphics& g, Vec2 center, int facing, float t, float scale, float alpha, const PhantomStyle& style) {
  const art::Assets& a = art::A();
  const int ps = PixelSize(scale);
  const int frame = (static_cast<int>(t / 10.f)) % 2;
  const SpriteId id = style.happy ? a.phantomHappy[frame] : a.phantom[frame];
  const Vec2 sz = g.SpriteSize(id);
  const float fps = static_cast<float>(ps);
  const Vec2 sq = style.squash;
  if (std::fabs(sq.x - 1.f) < 0.02f && std::fabs(sq.y - 1.f) < 0.02f) {
    g.DrawSprite(id, center.x - sz.x * fps * 0.5f, center.y - sz.y * fps * 0.5f + Bob(t, ps), ps, facing < 0, alpha);
  } else {  // 발끝(아랫변 가운데)을 고정하고 늘이거나 찌그러뜨린다
    const float w = sz.x * fps * sq.x, h = sz.y * fps * sq.y;
    const float bottom = center.y + sz.y * fps * 0.5f + Bob(t, ps);
    g.DrawSpriteSized(id, center.x - w * 0.5f, bottom - h, w, h, facing < 0, alpha);
  }
  if (style.hat) DrawTreasureHat(g, PhantomHatAnchor(center, facing, t, scale, sq), scale, style.hatGlow, t, alpha);
}

Vec2 PhantomHatAnchor(Vec2 center, int facing, float t, float scale, Vec2 squash) {
  // 24x24 스프라이트에서 두 귀 사이 정수리(11.5열, 6행)에 챙이 걸친다. 스쿼시는 발끝(24행) 기준.
  const float ps = static_cast<float>(PixelSize(scale));
  const float col = facing < 0 ? 24.f - 11.5f : 11.5f;
  const float bottom = center.y + 12.f * ps + Bob(t, PixelSize(scale));
  return {center.x + (col - 12.f) * ps * squash.x, bottom - (24.f - 6.f) * ps * squash.y};
}

void DrawTreasureHat(Graphics& g, Vec2 bc, float scale, float glow, float t, float alpha) {
  const SpriteId id = art::A().treasureHat;
  const int ps = PixelSize(scale);
  const float fps = static_cast<float>(ps);
  const Vec2 sz = g.SpriteSize(id);
  const float x = std::floor(bc.x - sz.x * fps * 0.5f), y = std::floor(bc.y - sz.y * fps);
  if (glow > 0.f) {
    const float pulse = 0.75f + 0.25f * std::sin(t * 0.12f);
    g.Glow(bc.x, bc.y - sz.y * fps * 0.5f, sz.x * fps * (0.9f + 0.3f * pulse), Rgb(255, 226, 122, static_cast<int>(200.f * glow * pulse * alpha)));
  }
  g.DrawSprite(id, x, y, ps, false, alpha);
  if (glow <= 0.f) return;
  // 모자 주위를 도는 십자 반짝이 (도트)
  for (int i = 0; i < 5; ++i) {
    const float ang = t * 0.05f + static_cast<float>(i) * kPi * 0.4f;
    const float tw = std::sin(t * 0.2f + static_cast<float>(i) * 1.7f);
    if (tw < 0.f) continue;
    const float sx = std::floor((bc.x + std::cos(ang) * sz.x * fps * 0.75f) / fps) * fps;
    const float sy = std::floor((bc.y - sz.y * fps * 0.5f + std::sin(ang) * sz.y * fps * 0.7f) / fps) * fps;
    const Color c = Rgb(255, 255, 255, static_cast<int>(255.f * glow * tw * alpha));
    g.FillRect(sx, sy - fps, fps, fps * 3.f, c);
    g.FillRect(sx - fps, sy, fps * 3.f, fps, c);
  }
}

void DrawShadowBall(Graphics& g, Vec2 c, float r, float t) {
  if (r < 3.f) return;
  const art::Assets& a = art::A();
  const int ps = Clamp(static_cast<int>(std::lround(r * 2.f / 8.f)), 1, 4);
  const SpriteId id = a.ball[(static_cast<int>(t / 4.f)) % 2];
  g.DrawSprite(id, c.x - 4.f * static_cast<float>(ps), c.y - 4.f * static_cast<float>(ps), ps);
}

}  // namespace gengar

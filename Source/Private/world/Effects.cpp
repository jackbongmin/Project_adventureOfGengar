#include "world/Effects.h"

#include <algorithm>
#include <cmath>

#include "art/Art.h"
#include "core/Config.h"
#include "core/Random.h"
#include "platform/Graphics.h"

namespace gengar {

namespace {
// 무너지는 발판 조각: 동굴 테마의 발판 도트를 그대로 떨어뜨린다.
void g_chunkDraw(Graphics& gfx, Vec2 topLeft) { gfx.DrawSprite(art::A().tiles[1].crumble, topLeft.x, topLeft.y, art::kPixel); }
}  // namespace

void Effects::Burst(Vec2 p, int count, Color color, float speed, float life) {
  for (int i = 0; i < count; ++i) {
    const float a = RandRange(0.f, 2.f * kPi), s = RandRange(0.3f, 1.f) * speed;
    const float l = life * RandRange(0.5f, 1.f);
    particles_.push_back({p, {std::cos(a) * s, std::sin(a) * s - 1.f}, l, life, RandRange(2.f, 5.f), color, 0.1f});
  }
}

void Effects::Trail(Vec2 p, Color color, float size, float life) {
  particles_.push_back({p, {RandRange(-0.3f, 0.3f), RandRange(-0.3f, 0.3f)}, life, life, size, color, 0.f});
}

void Effects::Dust(Vec2 p, int count, float pushX) {
  for (int i = 0; i < count; ++i) {
    const float life = RandRange(14.f, 24.f);
    const float size = std::floor(RandRange(2.f, 4.f)) * 2.f;  // 도트 느낌이 나도록 짝수 크기
    particles_.push_back({{p.x + RandRange(-6.f, 6.f), p.y - RandRange(0.f, 3.f)},
                          {RandRange(-1.f, 1.f) + pushX * RandRange(0.6f, 1.4f), RandRange(-1.1f, -0.3f)},
                          life, life, size, Rgb(214, 198, 240), 0.04f});
  }
}

void Effects::Pop(Vec2 p, const std::string& text, Color color) { texts_.push_back({p, text, color, 45}); }
void Effects::AddRing(Vec2 center, float reach) {
  const int life = reach >= 400.f ? 36 : 14;
  rings_.push_back({center, 10.f, (reach - 10.f) / static_cast<float>(life), life, life});
}
void Effects::AddChunk(Vec2 topLeft) { chunks_.push_back({topLeft, 0.f, 0.f, RandRange(-0.1f, 0.1f)}); }

void Effects::Update() {
  for (Particle& p : particles_) {
    p.p += p.v;
    p.v.y += p.gravity;
    p.v.x *= 0.98f;
    p.life -= 1.f;
  }
  particles_.erase(std::remove_if(particles_.begin(), particles_.end(), [](const Particle& p) { return p.life <= 0.f; }),
                   particles_.end());

  for (FloatText& t : texts_) { t.p.y -= 0.6f; --t.life; }
  texts_.erase(std::remove_if(texts_.begin(), texts_.end(), [](const FloatText& t) { return t.life <= 0; }), texts_.end());

  for (Ring& r : rings_) { r.r += r.grow; --r.life; }
  rings_.erase(std::remove_if(rings_.begin(), rings_.end(), [](const Ring& r) { return r.life <= 0; }), rings_.end());

  for (Chunk& c : chunks_) { c.vy += 0.5f; c.p.y += c.vy; c.rot += c.vr; }
  chunks_.erase(std::remove_if(chunks_.begin(), chunks_.end(),
                               [](const Chunk& c) { return c.p.y > static_cast<float>(cfg::kScreenH) + 60.f; }),
                chunks_.end());

  shake_ *= 0.88f;
  if (shake_ < 0.3f) shake_ = 0.f;
  if (flash_ > 0) --flash_;
}

void Effects::Render(Graphics& gfx) const {
  for (const Chunk& c : chunks_) g_chunkDraw(gfx, c.p);
  for (const Ring& r : rings_) {
    const float a = static_cast<float>(r.life) / static_cast<float>(r.maxLife);
    gfx.PixelRing(r.c.x, r.c.y, r.r, r.maxLife < 36 ? 4.f : 8.f, Rgb(210, 150, 255, static_cast<int>(a * 255)), 4.f);
  }
  for (const Particle& p : particles_) {
    const float a = std::max(0.f, p.life / p.maxLife);
    gfx.FillRect(p.p.x - p.size * 0.5f, p.p.y - p.size * 0.5f, p.size, p.size, WithAlpha(p.color, a));
  }
  for (const FloatText& t : texts_) {
    const float a = std::min(1.f, static_cast<float>(t.life) / 20.f);
    gfx.SetGlobalAlpha(a);
    gfx.Text(t.text, t.p.x, t.p.y, 2, t.color, Align::Center);
    gfx.SetGlobalAlpha(1.f);
  }
}

void Effects::Clear() {
  particles_.clear();
  texts_.clear();
  rings_.clear();
  chunks_.clear();
  shake_ = 0.f;
  flash_ = 0;
}

}  // namespace gengar

#include "actor/projectile/EnemyOrb.h"

#include <cmath>

#include "actor/Player.h"
#include "core/Config.h"
#include "art/Art.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

EnemyOrb::EnemyOrb(Vec2 center, Vec2 velocity, float homingTurn, int lifeFrames)
    : Actor(ActorKind::EnemyShot), homingTurn_(homingTurn), life_(lifeFrames) {
  pos = center;
  vel = velocity;
  size = {radius_ * 2.f, radius_ * 2.f};
}

void EnemyOrb::Update(World& w) {
  if (homingTurn_ > 0.f) {  // 유도: 속도는 유지하고 방향만 플레이어 쪽으로 조금씩 튼다
    const Vec2 pc = w.GetPlayer().Bounds().Center();
    const float speed = vel.Length();
    float delta = std::atan2(pc.y - pos.y, pc.x - pos.x) - std::atan2(vel.y, vel.x);
    while (delta > kPi) delta -= 2.f * kPi;
    while (delta < -kPi) delta += 2.f * kPi;
    const float ang = std::atan2(vel.y, vel.x) + Clamp(delta, -homingTurn_, homingTurn_);
    vel = {std::cos(ang) * speed, std::sin(ang) * speed};
    if (--life_ <= 0) { w.Fx().Burst(pos, 8, Hex(0xff5ca8), 3.f, 18.f); Kill(); return; }
  }
  pos += vel;
  const float T = static_cast<float>(cfg::kTile);
  if (pos.x < -40.f || pos.x > w.GetLevel().PixelWidth() + 40.f || pos.y < -40.f ||
      pos.y > static_cast<float>(cfg::kScreenH) + 40.f) {
    Kill();
    return;
  }
  if (w.GetLevel().Solid(static_cast<int>(std::floor(pos.x / T)), static_cast<int>(std::floor(pos.y / T)))) {
    w.Fx().Burst(pos, 5, Hex(0xff5ca8), 2.f, 16.f);
    Kill();
    return;
  }
  Player& p = w.GetPlayer();
  if (p.IsAlive()) {
    const Vec2 pc = p.Bounds().Center();
    if (std::hypot(pc.x - pos.x, pc.y - pos.y) < radius_ + 11.f) {
      p.TakeDamage(w, DamageInfo{1.f, Sign(vel.x) != 0 ? Sign(vel.x) : 1, false, false, "orb"});
      w.Fx().Burst(pos, 8, Hex(0xff5ca8), 3.f, 16.f);
      Kill();
      return;
    }
  }
  if (w.Frame() % 3 == 0) w.Fx().Trail(pos, Hex(0xff3c8c), 4.f, 12.f);
}

void EnemyOrb::Render(Graphics& g, const World&) {
  const SpriteId id = art::A().orb;
  const Vec2 sz = g.SpriteSize(id);
  g.DrawSprite(id, pos.x - sz.x * art::kPixel * 0.5f, pos.y - sz.y * art::kPixel * 0.5f, art::kPixel);
}

}  // namespace gengar

#include "actor/projectile/ShadowBall.h"

#include <cmath>

#include "actor/PhantomArt.h"
#include "actor/boss/Boss.h"
#include "actor/enemy/Enemy.h"
#include "core/Config.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

ShadowBall::ShadowBall(Vec2 center, Vec2 velocity, float radius, float damage, int life)
    : Actor(ActorKind::PlayerShot), radius_(radius), damage_(damage), life_(life) {
  pos = center;
  vel = velocity;
  size = {radius * 2.f, radius * 2.f};
}

void ShadowBall::Update(World& w) {
  pos += vel;
  ++age_;
  const Color puff = Hex(0x9a5cff);
  if (--life_ <= 0) { w.Fx().Burst(pos, 6, puff, 2.f, 14.f); Kill(); return; }

  const float T = static_cast<float>(cfg::kTile);
  if (w.GetLevel().Solid(static_cast<int>(std::floor(pos.x / T)), static_cast<int>(std::floor(pos.y / T)))) {
    w.Fx().Burst(pos, 8, puff, 3.f, 20.f);
    Kill();
    return;
  }

  const Rect box = Bounds();
  bool hit = false;
  w.ForEach(ActorKind::Enemy, [&](Actor& a) {
    if (hit || !a.Bounds().Overlaps(box)) return;
    static_cast<Enemy&>(a).TakeDamage(w, DamageInfo{damage_, Sign(vel.x)});
    hit = true;
  });
  if (!hit) {
    if (Boss* boss = w.GetBoss()) {
      if (boss->Bounds().Overlaps(box)) {
        boss->TakeDamage(w, DamageInfo{damage_, Sign(vel.x)});
        hit = true;
      }
    }
  }
  if (hit) {
    w.Fx().Burst(pos, 8, puff, 3.f, 20.f);
    Kill();
    return;
  }
  if (age_ % 2 == 0) w.Fx().Trail(pos, Hex(0x7b3fd0), radius_ * 0.7f, 14.f);
}

void ShadowBall::Render(Graphics& gfx, const World&) {
  DrawShadowBall(gfx, pos, radius_ * std::min(1.f, static_cast<float>(life_) / 10.f), static_cast<float>(age_));
}

}  // namespace gengar

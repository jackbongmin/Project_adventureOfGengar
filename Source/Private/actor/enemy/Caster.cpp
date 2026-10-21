#include "actor/enemy/Caster.h"

#include <cmath>

#include "actor/projectile/EnemyOrb.h"
#include "controller/TurretController.h"
#include "core/Random.h"
#include "platform/Audio.h"
#include "art/Art.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

Caster::Caster(Vec2 p, int stage)
    : Enemy(p, {26.f, 32.f}, 2 + stage), orbSpeed_(3.f + 0.4f * static_cast<float>(stage)) {
  SetController(std::make_unique<TurretController>(120 - 15 * stage, 80 + RandInt(0, 40)));
  facing_ = -1;
}

void Caster::OnShoot(World& w, Vec2 aim) {
  const Vec2 origin{pos.x + size.x * 0.5f + static_cast<float>(facing_) * 10.f, pos.y + 8.f};
  w.Spawn<EnemyOrb>(origin, Vec2{aim.x * orbSpeed_, aim.y * orbSpeed_});
  w.Snd().Play(Sfx::BossShoot);
}

void Caster::Render(Graphics& g, const World&) {
  const art::Assets& a = art::A();
  const SpriteId id = a.caster[telegraph_ > 0.4f ? 1 : 0];  // 발사 직전에는 지팡이 구슬이 밝아진다
  const Vec2 sz = g.SpriteSize(id), c = Bounds().Center();
  const float x = c.x - sz.x * art::kPixel * 0.5f, y = pos.y + size.y - sz.y * art::kPixel;
  g.DrawSprite(id, x, y, art::kPixel, facing_ < 0);
  if (flash_ > 0) g.DrawSprite(id, x, y, art::kPixel, facing_ < 0, 1.f, true);
}

}  // namespace gengar

#include "actor/projectile/Shockwave.h"

#include "actor/Player.h"
#include "core/Config.h"
#include "core/Random.h"
#include "art/Art.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

Shockwave::Shockwave(Vec2 center, float speedX, int lifeFrames) : Actor(ActorKind::EnemyShot), life_(lifeFrames) {
  pos = center;
  vel = {speedX, 0.f};
  size = {26.f, 22.f};
}

void Shockwave::Update(World& w) {
  pos += vel;
  if (life_ > 0 && --life_ == 0) { w.Fx().Burst(pos, 6, Hex(0xc58bff), 2.f, 12.f); Kill(); return; }
  const float T = static_cast<float>(cfg::kTile);
  if (pos.x < T + 8.f || pos.x > w.GetLevel().PixelWidth() - T - 8.f) { Kill(); return; }
  Player& p = w.GetPlayer();
  if (p.IsAlive() && Bounds().Overlaps(p.Bounds())) p.TakeDamage(w, DamageInfo{1.f, Sign(vel.x), false, false, "shockwave"});
  if (w.Frame() % 2 == 0) w.Fx().Trail({pos.x, pos.y + RandRange(-8.f, 8.f)}, Hex(0xc58bff), 4.f, 14.f);
}

void Shockwave::Render(Graphics& g, const World& w) {
  const SpriteId id = art::A().wave[(w.Frame() / 4) % 2];
  const Vec2 sz = g.SpriteSize(id);
  g.DrawSprite(id, pos.x - sz.x * art::kPixel * 0.5f, pos.y + 11.f - sz.y * art::kPixel, art::kPixel);
}

}  // namespace gengar

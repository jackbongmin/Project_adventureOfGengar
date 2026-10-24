#include "actor/enemy/Slime.h"

#include <cmath>

#include "actor/Player.h"
#include "platform/Audio.h"
#include "controller/PatrolController.h"
#include "art/Art.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

Slime::Slime(Vec2 p, int stage) : Enemy(p, {28.f, 22.f}, 26 + 10 * stage), stage_(stage) {
  moveSpeed_ = 0.8f + 0.2f * static_cast<float>(stage);
  SetController(std::make_unique<PatrolController>(-1));
  facing_ = -1;
}

void Slime::ApplyIntent(World& w) {
  if (warn_ > 0) { vel.x = 0.f; return; }   // 예고: 멈춰서 떤다
  if (hop_) { vel.x = hopVx_; return; }
  Enemy::ApplyIntent(w);
}

void Slime::UpdateEliteSkill(World& w) {
  if (hop_) {
    if (onGround_ && vel.y >= 0.f) { hop_ = false; w.Fx().Dust({pos.x + size.x * 0.5f, pos.y + size.y}, 8); w.Fx().Shake(2.f); }
    return;
  }
  if (warn_ == 1 && onGround_) {
    const float dx = w.GetPlayer().Bounds().Center().x - Bounds().Center().x;
    hopVx_ = Clamp(dx / 45.f, -3.5f, 3.5f);
    vel.y = -10.5f;
    onGround_ = false;
    hop_ = true;
    w.Snd().Play(Sfx::Jump);
    return;
  }
  if (warn_ == 0 && ++skillTimer_ >= 180) { skillTimer_ = 0; warn_ = 30; }
}

void Slime::OnDeath(World& w) {
  if (!Elite()) return;
  for (int k = -1; k <= 1; ++k) {
    Slime* m = w.Spawn<Slime>(Vec2{pos.x + static_cast<float>(k) * 10.f, pos.y}, stage_);
    m->hp_ = m->maxHp_ = 16;
    m->arenaTag = arenaTag;
    m->vel = {static_cast<float>(k) * 2.5f, -6.f};
    m->facing_ = k < 0 ? -1 : 1;
  }
  w.Fx().Pop({pos.x + size.x * 0.5f, pos.y - 24.f}, "SPLIT!", Hex(0x6df0a0));
}

void Slime::Render(Graphics& g, const World& w) {
  const art::Assets& a = art::A();
  const SpriteId id = a.slime[(w.Frame() / 14) % 2];
  const Vec2 sz = g.SpriteSize(id);
  const float x = pos.x + size.x * 0.5f - sz.x * art::kPixel * 0.5f, y = pos.y + size.y - sz.y * art::kPixel;
  g.DrawSprite(id, x, y, art::kPixel);
  if (flash_ > 0) g.DrawSprite(id, x, y, art::kPixel, false, 1.f, true);
}

}  // namespace gengar

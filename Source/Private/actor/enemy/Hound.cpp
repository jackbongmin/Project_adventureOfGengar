#include "actor/enemy/Hound.h"

#include <cmath>

#include "actor/Player.h"
#include "controller/ChaseController.h"
#include "platform/Audio.h"
#include "art/Art.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

Hound::Hound(Vec2 p, int stage) : Enemy(p, {34.f, 24.f}, 36 + 10 * stage) {
  moveSpeed_ = 2.1f + 0.25f * static_cast<float>(stage);  // 추적 속도
  jumpSpeed_ = 9.5f;
  auto chase = std::make_unique<ChaseController>(0.7f / moveSpeed_);  // 순찰 속도 0.7
  chase_ = chase.get();
  SetController(std::move(chase));
  facing_ = -1;
}

void Hound::ForceAlert() { if (chase_) chase_->ForceAggro(); }

void Hound::ApplyIntent(World& w) {
  if (warn_ > 0) {  // 예고: 멈춰서 플레이어 쪽을 노려본다
    vel.x = 0.f;
    SetFacing(w.GetPlayer().Bounds().Center().x >= Bounds().Center().x ? 1 : -1);
    return;
  }
  if (lunge_ > 0 && AheadBlocked(w, lungeDir_)) lunge_ = 0;  // 덮치다가도 앞이 절벽이나 벽이면 멈춘다
  if (lunge_ > 0) { vel.x = static_cast<float>(lungeDir_) * 8.5f; return; }
  Enemy::ApplyIntent(w);
}

void Hound::UpdateEliteSkill(World& w) {
  if (lunge_ > 0) {
    --lunge_;
    if (lunge_ % 2 == 0) w.Fx().Trail(Bounds().Center(), Hex(0xffc93c), 8.f, 14.f);
    return;
  }
  if (warn_ == 1) {
    lungeDir_ = facing_;
    lunge_ = 24;
    vel.y = -4.f;
    w.Snd().Play(Sfx::BossDash);
    return;
  }
  const Vec2 to = w.GetPlayer().Bounds().Center() - Bounds().Center();
  if (warn_ == 0 && ++skillTimer_ >= 150 && std::fabs(to.x) < 320.f && std::fabs(to.y) < 80.f) { skillTimer_ = 0; warn_ = 30; }
}

void Hound::Render(Graphics& g, const World& w) {
  const art::Assets& a = art::A();
  const int frame = (w.Frame() / (alerted_ ? 4 : 8)) % 2;  // 추적 중에는 빠르게 달린다
  const SpriteId id = alerted_ ? a.houndAlert[frame] : a.houndCalm[frame];
  const Vec2 sz = g.SpriteSize(id), c = Bounds().Center();
  const float x = c.x - sz.x * art::kPixel * 0.5f, y = pos.y + size.y - sz.y * art::kPixel;
  g.DrawSprite(id, x, y, art::kPixel, facing_ < 0);
  if (flash_ > 0) g.DrawSprite(id, x, y, art::kPixel, facing_ < 0, 1.f, true);
}

}  // namespace gengar

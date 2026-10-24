#include "actor/enemy/Bat.h"

#include <algorithm>
#include <cmath>

#include "actor/Player.h"
#include "platform/Audio.h"
#include "controller/FlyController.h"
#include "core/Random.h"
#include "art/Art.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

Bat::Bat(Vec2 p, int stage, float range) : Enemy(p, {26.f, 20.f}, 16 + 10 * stage), flapPhase_(RandRange(0.f, 6.f)) {
  SetController(std::make_unique<FlyController>(range, 0.f));
}

void Bat::ApplyIntent(World& w) {
  if (dive_ > 0) { vel = diveVel_; return; }
  if (warn_ > 0) { vel = {0.f, 0.f}; return; }  // 급강하 예고: 멈춰서 떤다
  if (chaseSpeed_ > 0.f) {  // 추적: 플레이어 쪽으로 곧게, 위아래로 살짝 너울거린다
    const Vec2 to = w.GetPlayer().Bounds().Center() - Bounds().Center();
    const float d = std::max(1.f, std::hypot(to.x, to.y));
    vel = {to.x / d * chaseSpeed_, to.y / d * chaseSpeed_ + std::sin(flapPhase_ + static_cast<float>(w.Frame()) * 0.12f) * 0.6f};
    SetFacing(to.x >= 0.f ? 1 : -1);
    return;
  }
  vel = intent_.flyVel;
}

// 정예 박쥐: 3초마다 0.5초 예고(멈춰서 "!") 뒤 플레이어가 있던 곳으로 빠르게 급강하했다가 돌아온다
void Bat::UpdateEliteSkill(World& w) {
  if (dive_ > 0) {
    if (--dive_ == 0) diveVel_ = {};
    else if (dive_ == 20) diveVel_ = {-diveVel_.x, -diveVel_.y};  // 반쯤 가면 되돌아 올라간다
    return;
  }
  if (warn_ == 1) {
    const Vec2 to = w.GetPlayer().Bounds().Center() - Bounds().Center();
    const float d = std::max(1.f, std::hypot(to.x, to.y));
    diveVel_ = {to.x / d * 6.5f, to.y / d * 6.5f};
    dive_ = 40;
    w.Snd().Play(Sfx::BossDash);
    return;
  }
  if (warn_ == 0 && ++skillTimer_ >= 180) { skillTimer_ = 0; warn_ = 30; }
}

void Bat::StepPhysics(World&) { pos += vel; }

void Bat::Render(Graphics& g, const World& w) {
  const art::Assets& a = art::A();
  const SpriteId id = a.bat[(w.Frame() / 5 + static_cast<int>(flapPhase_)) % 2];
  const Vec2 sz = g.SpriteSize(id), c = Bounds().Center();
  const float x = c.x - sz.x * art::kPixel * 0.5f, y = c.y - sz.y * art::kPixel * 0.5f;
  g.DrawSprite(id, x, y, art::kPixel);
  if (flash_ > 0) g.DrawSprite(id, x, y, art::kPixel, false, 1.f, true);
}

}  // namespace gengar

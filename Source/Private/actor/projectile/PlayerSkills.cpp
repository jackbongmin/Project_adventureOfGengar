#include "actor/projectile/PlayerSkills.h"

#include <cmath>

#include "actor/PhantomArt.h"
#include "actor/Player.h"
#include "actor/boss/Boss.h"
#include "actor/enemy/Enemy.h"
#include "actor/projectile/ShadowBall.h"
#include "core/Random.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {
namespace {

bool InCircle(const Actor& a, Vec2 c, float r) {
  const Rect b = a.Bounds();
  const float nx = Clamp(c.x, b.x, b.Right()), ny = Clamp(c.y, b.y, b.Bottom());
  return (nx - c.x) * (nx - c.x) + (ny - c.y) * (ny - c.y) < r * r;
}

void DamageAround(World& w, Vec2 c, float r, float enemyDmg, float bossDmg, bool ultimate) {
  DamageInfo info{enemyDmg, 1, false, ultimate};
  info.fromPlayer = true;
  w.ForEach(ActorKind::Enemy, [&](Actor& a) {
    if (!InCircle(a, c, r)) return;
    info.dir = a.Bounds().Center().x >= c.x ? 1 : -1;
    static_cast<Enemy&>(a).TakeDamage(w, info);
  });
  if (Boss* boss = w.GetBoss(); boss && !boss->dying && !boss->invulnerable && InCircle(*boss, c, r)) {
    info.amount = bossDmg;
    boss->TakeDamage(w, info);
  }
}

}  // namespace

// ---------------------------------------------------------------- DashBlast
DashBlast::DashBlast(Vec2 center, int facing, float damage, float radius)
    : Actor(ActorKind::PlayerShot), facing_(facing), damage_(damage), radius_(radius) {
  pos = center;
}

void DashBlast::Update(World& w) {
  if (++t_ < kDelay) return;
  DamageAround(w, pos, radius_, damage_, damage_, false);
  w.Fx().AddRing(pos, radius_);
  w.Fx().Burst(pos, 24, Hex(0xc58bff), 5.f, 24.f);
  w.Fx().Shake(4.f);
  w.Snd().Tone(180.f, 60.f, 0.18f, Wave::Saw, 0.07f);
  Kill();
}

void DashBlast::Render(Graphics& g, const World&) {
  const float k = static_cast<float>(t_) / static_cast<float>(kDelay);
  const bool blink = t_ > kDelay - 12 && (t_ / 2) % 2 == 0;  // 터지기 직전에 깜빡인다
  g.PixelRing(pos.x, pos.y, radius_ * k, 2.f, Rgb(255, 200, 255, static_cast<int>(120.f * k)), 2.f);
  DrawPhantom(g, pos, facing_, 0.f, 1.f, blink ? 0.9f : 0.45f);
}

// ---------------------------------------------------------------- BlackHole
BlackHole::BlackHole(Vec2 center, float enemyDamage, float bossDamage)
    : Actor(ActorKind::PlayerShot), enemyDamage_(enemyDamage), bossDamage_(bossDamage) {
  pos = center;
}

void BlackHole::Update(World& w) {
  ++t_;
  constexpr float kPull = 200.f, kHurt = 110.f;
  w.ForEach(ActorKind::Enemy, [&](Actor& a) {  // 끌어당기기 (걷는 적은 넉백 속도로, 그 외는 위치로)
    const Vec2 c = a.Bounds().Center();
    const float dx = pos.x - c.x, dy = pos.y - c.y, d = std::hypot(dx, dy);
    if (d > kPull || d < 4.f) return;
    static_cast<Enemy&>(a).Push(dx / d * 2.2f);
  });
  w.ForEach(ActorKind::EnemyShot, [&](Actor& a) {  // 적 탄환 흡수
    if (InCircle(a, pos, kHurt)) { w.Fx().Burst(a.pos, 4, Hex(0x5a2a98), 2.f, 10.f); a.Kill(); }
  });
  if (t_ % kTickFrames == 0) DamageAround(w, pos, kHurt, enemyDamage_, bossDamage_, true);
  if (t_ % 3 == 0) {  // 빨려 들어가는 도트
    const float a = RandRange(0.f, 2.f * kPi), r = RandRange(90.f, 160.f);
    w.Fx().Trail({pos.x + std::cos(a) * r, pos.y + std::sin(a) * r}, Hex(0x9a5cff), 4.f, 16.f);
  }
  if (t_ >= kLife) {
    w.Fx().Burst(pos, 30, Hex(0xc58bff), 6.f, 30.f);
    Kill();
  }
}

void BlackHole::Render(Graphics& g, const World&) {
  const float t = static_cast<float>(t_);
  const float grow = std::min(1.f, t / 20.f) * std::min(1.f, static_cast<float>(kLife - t_) / 20.f);
  g.Glow(pos.x, pos.y, 120.f * grow, Rgb(120, 40, 200, 70));
  g.FillCircle(pos.x, pos.y, 44.f * grow, Rgb(6, 0, 14, 240));
  for (int i = 0; i < 3; ++i) {  // 도는 고리
    const float r = (56.f + static_cast<float>(i) * 22.f) * grow;
    const float wob = std::sin(t * 0.2f + static_cast<float>(i)) * 3.f;
    g.PixelRing(pos.x, pos.y, r + wob, 3.f, Rgb(197, 139, 255, 150 - i * 40), 3.f);
  }
}

// ---------------------------------------------------------------- Familiar
Familiar::Familiar(Vec2 center) : Actor(ActorKind::Player) { pos = center; }

void Familiar::Update(World& w) {
  ++t_;
  const Player& p = w.GetPlayer();
  if (!p.IsAlive()) return;
  const Vec2 pc = p.Bounds().Center();
  const Vec2 want{pc.x - static_cast<float>(p.Facing()) * 30.f, pc.y - 30.f + std::sin(static_cast<float>(t_) * 0.08f) * 4.f};
  pos += (want - pos) * 0.12f;
  facing_ = p.Facing();

  if (--cooldown_ > 0) return;
  const Actor* best = nullptr;
  float bestD = 380.f;
  auto consider = [&](const Actor& a) {
    const Vec2 c = a.Bounds().Center();
    const float d = std::hypot(c.x - pos.x, c.y - pos.y);
    if (d < bestD) { best = &a; bestD = d; }
  };
  w.ForEach(ActorKind::Enemy, consider);
  if (const Boss* boss = w.GetBoss(); boss && boss->IsAlive() && !boss->dying && !boss->invulnerable) consider(*boss);
  if (!best) { cooldown_ = 10; return; }
  ShotSpec spec = p.MakeShot(w);  // 내 공의 카드 능력을 그대로 쓰고, 크기와 피해만 절반
  spec.damage *= 0.5f;
  spec.radius = std::max(5.f, spec.radius * 0.7f);
  spec.charged = false;
  const Vec2 c = best->Bounds().Center();
  const float a = std::atan2(c.y - pos.y, c.x - pos.x), speed = 7.f;
  w.SpawnPooled<ShadowBall>(pos, Vec2{std::cos(a) * speed, std::sin(a) * speed}, spec);
  facing_ = c.x >= pos.x ? 1 : -1;
  cooldown_ = kFireFrames;
}

void Familiar::Render(Graphics& g, const World&) {
  DrawPhantom(g, pos, facing_, static_cast<float>(t_), 0.55f, 0.9f);
}

}  // namespace gengar

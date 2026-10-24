#include "actor/projectile/PlayerSkills.h"

#include <cmath>

#include "actor/PhantomArt.h"
#include "actor/Player.h"
#include "actor/boss/Boss.h"
#include "actor/enemy/Enemy.h"
#include "actor/projectile/ShadowBall.h"
#include "core/Random.h"
#include "data/PlayerStats.h"
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
// 여러 마리면 쏘는 때를 16프레임씩 엇갈려 한꺼번에 몰리지 않게 한다
Familiar::Familiar(Vec2 center, int index) : Actor(ActorKind::Player), cooldown_(kFireFrames + 16 * index), index_(index) { pos = center; }

void Familiar::Update(World& w) {
  ++t_;
  const Player& p = w.GetPlayer();
  if (!p.IsAlive()) return;
  const Vec2 pc = p.Bounds().Center();
  // 플레이어 뒤로 26px 간격 한 줄. 뒤쪽 분신일수록 조금 늦게 따라와 꼬리처럼 흔들린다
  const float back = 30.f + 26.f * static_cast<float>(index_);
  const Vec2 want{pc.x - static_cast<float>(p.Facing()) * back, pc.y - 30.f + std::sin(static_cast<float>(t_) * 0.08f + static_cast<float>(index_) * 0.9f) * 4.f};
  pos += (want - pos) * (0.12f - 0.025f * static_cast<float>(index_));
  facing_ = p.Facing();
  if (--cooldown_ > 0) return;
  cooldown_ = Volley(w) ? kFireFrames : 10;  // 쏠 적이 없으면 잠깐 뒤 다시 본다
}

bool Familiar::Volley(World& w) {
  const Player& p = w.GetPlayer();
  if (!p.IsAlive()) return false;
  const Actor* best = nullptr;
  float bestD = 420.f;
  auto consider = [&](const Actor& a) {
    const Vec2 c = a.Bounds().Center();
    const float d = std::hypot(c.x - pos.x, c.y - pos.y);
    if (d < bestD) { best = &a; bestD = d; }
  };
  w.ForEach(ActorKind::Enemy, consider);
  if (const Boss* boss = w.GetBoss(); boss && boss->IsAlive() && !boss->dying && !boss->invulnerable) consider(*boss);
  if (!best) return false;
  ShotSpec spec = p.MakeShot(w);  // 내 공의 카드 능력을 그대로 쓰고, 크기 70%·피해 50%
  spec.damage *= 0.5f;
  spec.radius = std::max(5.f, spec.radius * 0.7f);
  spec.charged = false;
  const float aim = std::atan2(best->Bounds().Center().y - pos.y, best->Bounds().Center().x - pos.x);
  const float speed = spec.laser ? 18.f : 7.f;
  const int n = stats::ShotCount(w.Sess());
  for (int i = 0; i < n; ++i) {
    // 내 부채꼴과 같은 벌어짐을 조준 방향 기준으로 돌려서 쓴다
    const float a = aim + std::atan2(Player::SpreadY(i, n, w.Sess()), speed);
    w.SpawnPooled<ShadowBall>(pos, Vec2{std::cos(a) * speed, std::sin(a) * speed}, spec);
  }
  facing_ = std::cos(aim) >= 0.f ? 1 : -1;
  return true;
}

void Familiar::Render(Graphics& g, const World&) {
  DrawPhantom(g, pos, facing_, static_cast<float>(t_), 0.55f, 0.9f);
}

}  // namespace gengar

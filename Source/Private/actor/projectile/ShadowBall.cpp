#include "actor/projectile/ShadowBall.h"

#include <cmath>

#include "actor/PhantomArt.h"
#include "actor/boss/Boss.h"
#include "actor/enemy/Enemy.h"
#include "core/Config.h"
#include "core/Random.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {
namespace {

constexpr float kBurstRadius = 40.f;
constexpr float kHomingRange = 320.f;
constexpr float kHomingTurn = 0.11f;  // 프레임당 최대로 꺾는 각도 (라디안)

bool Hittable(const Actor& a) {
  if (a.Kind() == ActorKind::Boss) {
    const Boss& b = static_cast<const Boss&>(a);
    return b.IsAlive() && !b.dying && !b.invulnerable;
  }
  return !a.IsDead();
}

}  // namespace

ShadowBall::ShadowBall(Vec2 center, Vec2 velocity, const ShotSpec& spec, const Actor* ignore)
    : Actor(ActorKind::PlayerShot), spec_(spec), life_(spec.life) {
  pos = center;
  origin_ = center;
  vel = velocity;
  size = {spec.radius * 2.f, spec.radius * 2.f};
  if (ignore) hits_[hitCount_++] = ignore;
}

bool ShadowBall::AlreadyHit(const Actor* a) const {
  for (int i = 0; i < hitCount_; ++i) {
    if (hits_[i] == a) return true;
  }
  return false;
}

// 유도: 앞쪽 반원 안의 가장 가까운 적(또는 보스)을 향해 조금씩 꺾는다. 속력은 그대로.
void ShadowBall::Steer(World& w) {
  const Actor* best = nullptr;
  float bestD = kHomingRange;
  auto consider = [&](const Actor& a) {
    if (!Hittable(a) || AlreadyHit(&a)) return;
    const Vec2 c = a.Bounds().Center();
    const float dx = c.x - pos.x, dy = c.y - pos.y, d = std::hypot(dx, dy);
    if (d < bestD && dx * vel.x + dy * vel.y > -0.2f * d * std::hypot(vel.x, vel.y)) { best = &a; bestD = d; }
  };
  w.ForEach(ActorKind::Enemy, consider);
  if (const Boss* boss = w.GetBoss()) consider(*boss);
  if (!best) return;
  const Vec2 c = best->Bounds().Center();
  const float speed = std::hypot(vel.x, vel.y);
  const float cur = std::atan2(vel.y, vel.x), want = std::atan2(c.y - pos.y, c.x - pos.x);
  float diff = std::remainder(want - cur, 2.f * kPi);
  diff = Clamp(diff, -kHomingTurn, kHomingTurn);
  vel = {std::cos(cur + diff) * speed, std::sin(cur + diff) * speed};
}

void ShadowBall::Hit(World& w, Actor& target) {
  if (hitCount_ < kMaxHits) hits_[hitCount_++] = &target;
  float dmg = spec_.damage * (w.TimeStopped() ? 2.f : 1.f);
  const bool crit = !spec_.ultimate && spec_.crit > 0.f && Chance(spec_.crit);
  if (crit) dmg *= 2.f;
  DamageInfo info{dmg, vel.x >= 0.f ? 1 : -1, false, spec_.ultimate};
  info.fromPlayer = true;
  info.crit = crit;
  info.curse = spec_.curse;
  info.knockback = spec_.knockback;
  if (target.Kind() == ActorKind::Boss) {
    info.amount *= spec_.bossScale;
    static_cast<Boss&>(target).TakeDamage(w, info);
  } else {
    static_cast<Enemy&>(target).TakeDamage(w, info);
  }

  if (spec_.fragment) return;
  if (spec_.burst) {  // 폭발: 주변 적에게 50%
    DamageInfo splash = info;
    splash.amount = dmg * 0.5f;
    splash.crit = false;
    w.ForEach(ActorKind::Enemy, [&](Actor& a) {
      if (&a == &target) return;
      const Vec2 c = a.Bounds().Center();
      if (std::hypot(c.x - pos.x, c.y - pos.y) < kBurstRadius + a.size.x * 0.5f) static_cast<Enemy&>(a).TakeDamage(w, splash);
    });
    if (Boss* boss = w.GetBoss(); boss && boss != &target && boss->Bounds().Overlaps({pos.x - kBurstRadius, pos.y - kBurstRadius, kBurstRadius * 2.f, kBurstRadius * 2.f})) {
      splash.amount *= spec_.bossScale;
      boss->TakeDamage(w, splash);
    }
    w.Fx().AddRing(pos, kBurstRadius);
    w.Fx().Burst(pos, 14, Hex(0xffb347), 4.f, 18.f);
  }
  if (spec_.split) {  // 분열: 맞은 적은 건너뛰고 세 갈래로
    ShotSpec frag = spec_;
    frag.radius = std::max(4.f, spec_.radius * 0.55f);
    frag.damage = spec_.damage * 0.4f;
    frag.life = 26;
    frag.pierce = 0;
    frag.bounces = 0;
    frag.fragment = true;
    frag.charged = false;
    const float base = std::atan2(vel.y, vel.x), speed = std::max(6.f, std::hypot(vel.x, vel.y) * 0.8f);
    for (int k = -1; k <= 1; ++k) {
      const float a = base + static_cast<float>(k) * 0.9f;
      w.SpawnPooled<ShadowBall>(pos, Vec2{std::cos(a) * speed, std::sin(a) * speed}, frag, &target);
    }
  }
}

void ShadowBall::Update(World& w) {
  history_[age_ % kHistory] = pos;
  const Vec2 prev = pos;
  if (spec_.homing) Steer(w);
  pos += vel;
  ++age_;
  const Color puff = Hex(0x9a5cff);
  if (--life_ <= 0) { w.Fx().Burst(pos, 6, puff, 2.f, 14.f); Kill(); return; }

  const float T = static_cast<float>(cfg::kTile);
  const Level& lv = w.GetLevel();
  auto solid = [&](float x, float y) { return lv.Solid(static_cast<int>(std::floor(x / T)), static_cast<int>(std::floor(y / T))); };
  if (solid(pos.x, pos.y)) {
    if (spec_.bounces <= 0) {
      w.Fx().Burst(pos, 8, puff, 3.f, 20.f);
      Kill();
      return;
    }
    // 튕김: 어느 축으로 들어왔는지 보고 그 축의 속도를 뒤집는다
    --spec_.bounces;
    if (!solid(prev.x, pos.y)) vel.x = -vel.x;
    else if (!solid(pos.x, prev.y)) vel.y = -vel.y;
    else vel = {-vel.x, -vel.y};
    pos = prev;
    life_ = std::max(life_, 24);
    w.Fx().Burst(pos, 5, Hex(0xd9b8ff), 2.f, 10.f);
  }

  const Rect box = Bounds();
  bool stop = false;
  auto tryHit = [&](Actor& a) {
    if (stop || AlreadyHit(&a) || !Hittable(a) || !a.Bounds().Overlaps(box)) return;
    Hit(w, a);
    if (spec_.pierce > 0) --spec_.pierce;
    else stop = true;
  };
  w.ForEach(ActorKind::Enemy, tryHit);
  if (!stop) {
    if (Boss* boss = w.GetBoss()) {
      if (!boss->invulnerable && !boss->dying && !AlreadyHit(boss) && boss->Bounds().Overlaps(box)) {
        Hit(w, *boss);
        if (spec_.pierce > 0) --spec_.pierce;
        else stop = true;
      }
    }
  }
  if (stop) {
    w.Fx().Burst(pos, 8, puff, 3.f, 20.f);
    Kill();
    return;
  }
  if (age_ % 2 == 0) w.Fx().Trail(pos, spec_.curse ? Hex(0x5a1a7a) : Hex(0x7b3fd0), spec_.radius * 0.7f, 14.f);
}

void ShadowBall::Render(Graphics& gfx, const World&) {
  const float t = static_cast<float>(age_);
  const float r = spec_.radius * std::min(1.f, static_cast<float>(life_) / 10.f);
  // 발사 임팩트: 출발점에서 퍼지며 옅어지는 도트 고리
  if (age_ < 7 && !spec_.fragment) {
    const float k = t / 7.f;
    gfx.PixelRing(origin_.x, origin_.y, spec_.radius * 0.8f + t * 3.f, 3.f, Rgb(225, 190, 255, static_cast<int>((1.f - k) * 220.f)), 3.f);
  }
  if (spec_.charged) gfx.Glow(pos.x, pos.y, r * 2.2f, Rgb(255, 226, 122, 90));
  // 잔상: 오래된 것부터 그려서 본체가 맨 위에 오게 한다
  for (int k = kGhosts; k >= 1; --k) {
    const int back = k * kGhostStride;
    if (back > age_) continue;
    const Vec2 p = history_[(age_ - back) % kHistory];
    DrawShadowBall(gfx, p, r * (1.f - 0.12f * static_cast<float>(k)), t, 0.5f - 0.13f * static_cast<float>(k));
  }
  DrawShadowBall(gfx, pos, r, t);
}

}  // namespace gengar

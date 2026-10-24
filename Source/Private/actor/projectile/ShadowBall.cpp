#include "actor/projectile/ShadowBall.h"

#include <cmath>

#include "actor/PhantomArt.h"
#include "actor/Player.h"
#include "actor/boss/Boss.h"
#include "actor/enemy/Enemy.h"
#include "core/Config.h"
#include "core/Random.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {
namespace {

constexpr float kBurstRadius = 64.f;
constexpr float kHomingRange = 320.f;
constexpr float kHomingTurn = 0.11f;  // 프레임당 최대로 꺾는 각도 (라디안)
constexpr float kSplitSpread = 0.9f;  // 분열 조각 사이 각도 (라디안)
constexpr int kFragmentLife = 48;     // 분열 조각 수명 (약 380px)
constexpr int kRehitDelay = 18;       // 튕긴 뒤 이만큼(보스 몸을 빠져나갈 시간) 지나면 이미 맞힌 적도 다시 맞힐 수 있다

bool Hittable(const Actor& a) {
  if (a.Kind() == ActorKind::Boss) {
    const Boss& b = static_cast<const Boss&>(a);
    return b.IsAlive() && !b.dying && !b.invulnerable;
  }
  return !a.IsDead();
}

}  // namespace

ShadowBall::ShadowBall(Vec2 center, Vec2 velocity, const ShotSpec& spec, const Actor* ignore)
    : Actor(ActorKind::PlayerShot), spec_(spec), life_(spec.life), bouncy_(spec.bounces > 0) {
  pos = center;
  origin_ = center;
  vel = velocity;
  size = {spec.radius * 2.f, spec.radius * 2.f};
  if (ignore) hits_[hitCount_++] = ignore;
}

// 맞힌 목록이 꽉 차면 더 맞히지 않는다 (기록 못 한 적을 매 프레임 다시 때리는 일 방지)
bool ShadowBall::AlreadyHit(const Actor* a) const {
  if (hitCount_ >= kMaxHits) return true;
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

// 분열: dir 방향을 가운데로 세 갈래. 공 하나당 한 번만 (적에게 맞거나 벽에 닿을 때).
// 튕기는 탄이 있으면 조각도 한 번 튕긴다. ignore 로 넘긴 적은 조각이 맞히지 않는다.
void ShadowBall::Split(World& w, Vec2 at, Vec2 dir, const Actor* ignore) {
  if (!spec_.split || spec_.fragment || splitDone_) return;
  splitDone_ = true;
  ShotSpec frag = spec_;
  frag.radius = std::max(4.f, spec_.radius * 0.55f);
  frag.damage = spec_.damage * 0.4f;
  frag.life = kFragmentLife;
  frag.pierce = 0;
  frag.bounces = bouncy_ ? 1 : 0;
  frag.fragment = true;
  frag.charged = false;
  frag.boomerang = false;  // 조각까지 돌아오면 같은 적을 몇 번이고 다시 맞힌다
  const float base = std::atan2(dir.y, dir.x), speed = std::max(6.f, std::hypot(dir.x, dir.y));
  for (int k = -1; k <= 1; ++k) {
    const float a = base + static_cast<float>(k) * kSplitSpread;
    w.SpawnPooled<ShadowBall>(at, Vec2{std::cos(a) * speed, std::sin(a) * speed}, frag, ignore);
  }
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
  info.poison = spec_.poison;
  info.frost = spec_.frost;
  if (target.Kind() == ActorKind::Boss) {
    info.amount *= spec_.bossScale;
    static_cast<Boss&>(target).TakeDamage(w, info);
  } else {
    static_cast<Enemy&>(target).TakeDamage(w, info);
  }

  if (spec_.chain) Zap(w, target, dmg * 0.5f);
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
    // 주황 고리가 폭발 범위 끝까지 퍼지고, 안쪽에 노란 고리와 불꽃이 함께 튄다
    w.Fx().AddRing(pos, kBurstRadius, Hex(0xffb347));
    w.Fx().AddRing(pos, kBurstRadius * 0.6f, Hex(0xffe27a));
    w.Fx().Burst(pos, 24, Hex(0xffb347), 5.5f, 22.f);
    w.Fx().Burst(pos, 10, Hex(0xfff0b0), 3.f, 14.f);
    w.Fx().Shake(1.5f);
  }
  // 분열: 일반 적은 건너뛰고 주변으로 퍼지고, 보스는 커다란 몸이라 조각 셋을 모두 맞는다
  Split(w, pos, vel, target.Kind() == ActorKind::Boss ? nullptr : &target);
}

// 연쇄 번개: from 을 뺀 가장 가까운 적(또는 보스) 하나에게 번개. 가는 길에 하늘색 도트가 튄다
void ShadowBall::Zap(World& w, const Actor& from, float damage) {
  const Vec2 a = from.Bounds().Center();
  Actor* best = nullptr;
  float bestD = 170.f;
  auto consider = [&](Actor& t) {
    if (&t == &from || !Hittable(t)) return;
    const Vec2 c = t.Bounds().Center();
    const float d = std::hypot(c.x - a.x, c.y - a.y);
    if (d < bestD) { bestD = d; best = &t; }
  };
  w.ForEach(ActorKind::Enemy, consider);
  if (Boss* boss = w.GetBoss()) consider(*boss);
  if (!best) return;
  const Vec2 b = best->Bounds().Center();
  for (int k = 1; k <= 7; ++k) {  // 지그재그 번개 자국
    const float t = static_cast<float>(k) / 8.f;
    w.Fx().Burst({a.x + (b.x - a.x) * t + RandRange(-6.f, 6.f), a.y + (b.y - a.y) * t + RandRange(-6.f, 6.f)}, 2, Hex(0x9ff0ff), 1.f, 10.f);
  }
  DamageInfo info{damage, b.x >= a.x ? 1 : -1};
  info.fromPlayer = true;
  if (best->Kind() == ActorKind::Boss) static_cast<Boss&>(*best).TakeDamage(w, info);
  else static_cast<Enemy&>(*best).TakeDamage(w, info);
}

// 튕김 한 번을 쓰고, 조금 뒤에는 이미 맞힌 적(보스 포함)도 다시 맞힐 수 있게 한다.
// 돌아오는 부메랑은 예외: 돌아오는 길에는 대상마다 딱 한 번만 맞힌다
void ShadowBall::UseBounce() {
  --spec_.bounces;
  life_ = std::max(life_, 30);
  if (!returning_) rehit_ = kRehitDelay;
}

void ShadowBall::Update(World& w) {
  history_[age_ % kHistory] = pos;
  const Vec2 prev = pos;
  if (spec_.homing) Steer(w);
  pos += vel;
  ++age_;
  const Color puff = Hex(0x9a5cff);
  const Vec2 prevShift = waveShift_;
  if (spec_.wave) {  // 웨이브: 진행 방향의 수직으로 사인파만큼 옮긴다 (진폭 16px, 화면 위쪽부터)
    const float sp = std::max(1.f, std::hypot(vel.x, vel.y));
    const Vec2 perp{-vel.y / sp, vel.x / sp};
    const float flip = perp.y > 0.f ? -1.f : 1.f;  // 처음에는 위로 출렁인다 (바닥 가까이에서 쏴도 땅에 박히지 않게)
    waveShift_ = perp * (flip * std::sin(static_cast<float>(age_) * 0.3f) * 16.f);
    pos += waveShift_ - prevShift;
  }
  if (spec_.boomerang) {  // 부메랑: 사거리 절반에서 돌아서 플레이어에게 날아온다. 돌아오는 길에 한 번 더 맞힐 수 있다
    const Vec2 pc = w.GetPlayer().Bounds().Center();
    const float sp = std::max(4.f, std::hypot(vel.x, vel.y));
    if (!returning_ && life_ <= spec_.life / 2) { returning_ = true; hitCount_ = 0; rehit_ = 0; life_ = std::max(life_, 90); }
    if (returning_) {
      const Vec2 to = pc - pos;
      const float d = std::max(1.f, std::hypot(to.x, to.y));
      vel = {to.x / d * sp, to.y / d * sp};
      if (d < 20.f) { Kill(); return; }
    }
  }
  if (--life_ <= 0) { w.Fx().Burst(pos, 6, puff, 2.f, 14.f); Kill(); return; }
  if (rehit_ > 0 && --rehit_ == 0) hitCount_ = 0;

  // 뜬 발판(OneWay)은 뚫고 지나간다. 지면/벽돌/벽에 닿으면 사라지거나 튕긴다.
  const float T = static_cast<float>(cfg::kTile);
  const Level& lv = w.GetLevel();
  auto solid = [&](float x, float y) {
    const int c = static_cast<int>(std::floor(x / T)), r = static_cast<int>(std::floor(y / T));
    return lv.Solid(c, r) && !lv.OneWay(c, r);
  };
  // 웨이브 공은 출렁임을 뺀 중심선으로 벽·바닥을 판단한다 (출렁임만으로 땅에 닿아 사라지지 않게)
  const Vec2 line = pos - waveShift_, prevLine = prev - prevShift;
  if (solid(line.x, line.y)) {
    // 어느 축으로 들어왔는지 보고 그 축의 속도를 뒤집은 방향 = 벽에서 튀어나오는 방향
    Vec2 out = vel;
    if (!solid(prevLine.x, line.y)) out.x = -out.x;
    else if (!solid(line.x, prevLine.y)) out.y = -out.y;
    else out = {-out.x, -out.y};
    Split(w, prev, out, nullptr);  // 분열탄은 벽에 닿아도 갈라진다
    if (spec_.bounces <= 0) {
      w.Fx().Burst(pos, 8, puff, 3.f, 20.f);
      Kill();
      return;
    }
    vel = out;
    pos = prev;
    UseBounce();
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
  // 튕기는 탄: 관통이 다 떨어져도 튕김이 남아 있으면 맞힌 자리에서 아무 방향으로나 튀어 나간다
  auto ricochet = [&]() {
    if (!stop || spec_.bounces <= 0) return;
    stop = false;
    const float a = RandRange(0.f, 2.f * kPi), speed = std::hypot(vel.x, vel.y);
    vel = {std::cos(a) * speed, std::sin(a) * speed};
    UseBounce();
    w.Fx().Burst(pos, 6, Hex(0xd9b8ff), 2.5f, 12.f);
  };
  w.ForEach(ActorKind::Enemy, tryHit);
  ricochet();
  if (!stop) {
    if (Boss* boss = w.GetBoss()) {
      if (!boss->invulnerable && !boss->dying && !AlreadyHit(boss) && boss->Bounds().Overlaps(box)) {
        Hit(w, *boss);
        if (spec_.pierce > 0) --spec_.pierce;
        else stop = true;
        ricochet();
      }
    }
  }
  if (stop) {
    w.Fx().Burst(pos, 8, puff, 3.f, 20.f);
    Kill();
    return;
  }
  if (age_ % 2 == 0 && !spec_.laser) {
    const Color trail = spec_.poison ? Hex(0x3a9a4a) : spec_.frost ? Hex(0x3a8ab0) : spec_.curse ? Hex(0x5a1a7a) : Hex(0x7b3fd0);
    w.Fx().Trail(pos, trail, spec_.radius * 0.7f, 14.f);
  }
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
  if (spec_.laser) {  // 레이저: 지나온 길을 따라 늘어진 빛줄기 (바깥 분홍, 가운데 흰색)
    const float sp = std::max(1.f, std::hypot(vel.x, vel.y));
    const Vec2 d{vel.x / sp, vel.y / sp};
    const float len = std::min(sp * 4.f, static_cast<float>(age_) * sp);
    gfx.PixelLine(pos - d * len, pos, std::max(4.f, r * 1.1f), Rgb(255, 90, 200, 170), 2.f);
    gfx.PixelLine(pos - d * (len * 0.8f), pos, std::max(2.f, r * 0.45f), Hex(0xffffff), 2.f);
    gfx.Glow(pos.x, pos.y, r * 1.6f, Rgb(255, 120, 220, 80));
    return;
  }
  if (spec_.poison) gfx.Glow(pos.x, pos.y, r * 1.8f, Rgb(90, 230, 110, 70));
  if (spec_.frost) gfx.Glow(pos.x, pos.y, r * 1.8f, Rgb(120, 220, 255, 70));
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

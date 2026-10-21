#include "actor/boss/Boss.h"

#include <cmath>
#include <string>

#include "actor/PhantomArt.h"
#include "actor/Player.h"
#include "actor/enemy/Enemy.h"
#include "actor/projectile/EnemyOrb.h"
#include "art/Art.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "data/BossTuning.h"
#include "core/Random.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "state/boss/BossStates.h"
#include "world/World.h"

namespace gengar {

Boss::Boss(Vec2 center, int maxHp) : Actor(ActorKind::Boss), machine_(*this), hp_(static_cast<float>(maxHp)), maxHp_(static_cast<float>(maxHp)) {
  pos = center;
  size = {104.f, 104.f};
  machine_.Change(MakeBossEnterState());
  machine_.Apply();
}

float Boss::PhaseSpeed() const { return Tuning(phase_).moveSpeed; }

Vec2 Boss::PlayerCenter() const { return world_->GetPlayer().Bounds().Center(); }

Vec2 Boss::HatHandPos() const {
  const float t = static_cast<float>(tick_);
  const Vec2 rest{pos.x + 98.f + std::sin(t * 0.05f + 1.f) * 8.f, pos.y + 30.f + std::cos(t * 0.06f + 1.f) * 10.f};
  return {rest.x + (handTarget.x - rest.x) * handReach, rest.y + (handTarget.y - rest.y) * handReach};
}

void Boss::ToFloat() {
  charge = 0.f;
  beam.active = false;
  alpha = 1.f;
  invulnerable = false;
  contactEnabled = false;  // 평소에는 몸으로 피해를 주지 않는다
  damageMultiplier = 1.f;
  machine_.Change(MakeBossFloatState());
}

void Boss::StartAttack(BossAttack attack) {
  attackName_ = gengar::AttackName(attack);
  DebugLog("[boss] attack %s (phase %d, hp %d/%d)", attackName_, phase_ + 1, Hp(), MaxHp());
  attackNameTimer_ = 70;
  machine_.Change(MakeBossAttackState(attack));
}

// 플레이어를 몸으로 쫓지 않고, 약 250px 거리를 둔 채 둥실 떠다닌다. 벽에 몰리면 반대편으로 넘어간다.
void Boss::FloatMove() {
  constexpr float kW = static_cast<float>(cfg::kScreenW), kKeepDistance = 230.f;
  const float pc = PlayerCenter().x, dx = pos.x - pc;
  if (std::fabs(dx) > 80.f) sideDir_ = dx > 0.f ? 1 : -1;
  float desired = pc + static_cast<float>(sideDir_) * kKeepDistance;
  if (desired > kW - 110.f || desired < 110.f) {
    sideDir_ = -sideDir_;
    desired = pc + static_cast<float>(sideDir_) * kKeepDistance;
  }
  desired = Clamp(desired + std::sin(static_cast<float>(tick_) * 0.02f) * 40.f, 110.f, kW - 110.f);
  const float sp = PhaseSpeed();
  pos.x += Clamp((desired - pos.x) * 0.05f, -sp, sp);
  pos.y += (330.f + std::sin(static_cast<float>(tick_) * 0.04f) * 45.f - pos.y) * 0.06f;
}

void Boss::FireHomingOrb(Vec2 from, float angle, float speed, float turnPerFrame, int lifeFrames) {
  world_->Spawn<EnemyOrb>(from, Vec2{std::cos(angle) * speed, std::sin(angle) * speed}, turnPerFrame, lifeFrames);
}

void Boss::FireOrb(Vec2 from, float angle, float speed) {
  world_->Spawn<EnemyOrb>(from, Vec2{std::cos(angle) * speed, std::sin(angle) * speed});
}

void Boss::Update(World& w) {
  world_ = &w;
  ++tick_;
  if (flash_ > 0) --flash_;
  if (attackNameTimer_ > 0) --attackNameTimer_;

  const Vec2 pc = PlayerCenter();
  look_ = {Clamp((pc.x - pos.x) / 20.f, -1.f, 1.f) * 7.f, Clamp((pc.y - pos.y) / 20.f, -1.f, 1.f) * 6.f};

  if (!dying) {  // 체력 66% / 33% 에서 단계 상승
    const float ratio = hp_ / maxHp_;
    const int ph = ratio > 0.66f ? 0 : ratio > 0.33f ? 1 : 2;
    if (ph != phase_) {
      phase_ = ph;
      DebugLog("[boss] PHASE %d reached (hp %d/%d)", ph + 1, Hp(), MaxHp());
      w.Fx().Shake(14.f);
      w.Snd().Play(Sfx::BossPhase);
      w.ShowBanner(ph == 1 ? "- RAGE -" : "- FURY -", 90);
    }
  }

  if (tick_ % 120 == 0) DebugLog("[boss] state=%s dist=%d", machine_.CurrentName(), static_cast<int>(std::fabs(pos.x - pc.x)));
  machine_.Update();
  pos.x = Clamp(pos.x, 90.f, static_cast<float>(cfg::kScreenW) - 90.f);

  if (contactEnabled && !dying && alpha > 0.5f) {
    Player& p = w.GetPlayer();
    if (p.IsAlive() && Bounds().Overlaps(p.Bounds())) {
      const int dir = Sign(p.Bounds().Center().x - pos.x);
      p.TakeDamage(w, DamageInfo{1.f, dir != 0 ? dir : 1, false, false, "boss-body"});
    }
  }
}

void Boss::TakeDamage(World& w, const DamageInfo& info) {
  if (invulnerable || dying || Debug().immortal) { w.Snd().Play(Sfx::BossHit); return; }
  w.AddGauge(3.f);
  const float dmg = info.amount * damageMultiplier;
  hp_ -= dmg;
  flash_ = 5;
  w.Fx().Pop({pos.x + RandRange(-30.f, 30.f), pos.y - 60.f}, "-" + std::to_string(static_cast<int>(std::lround(dmg))), Hex(0xffd0ff));
  w.Snd().Play(Sfx::BossHit);
  if (hp_ > 0.f) return;

  hp_ = 0.f;
  beam.active = false;
  w.ForEach(ActorKind::EnemyShot, [](Actor& a) { a.Kill(); });
  w.ForEach(ActorKind::Hazard, [](Actor& a) { a.Kill(); });
  w.ForEach(ActorKind::Enemy, [&](Actor& a) { static_cast<Enemy&>(a).TakeDamage(w, DamageInfo{999.f, 1, false, true}); });
  w.Snd().Play(Sfx::BossDeath);
  machine_.Change(MakeBossDyingState());
  machine_.Apply();
}

void Boss::Render(Graphics& g, const World&) {
  const art::Assets& a = art::A();
  const float t = static_cast<float>(tick_);
  const int ps = art::kPixel;
  const Color accent = Hsl(phase_ == 0 ? 272.f : phase_ == 1 ? 310.f : 355.f, 0.9f, 0.62f);

  g.SetGlobalAlpha(alpha);

  // 기를 모을 때: 몸 주위로 모여드는 도트 링
  if (charge > 0.05f) g.PixelRing(pos.x, pos.y, 118.f - charge * 30.f, 4.f, WithAlpha(accent, 0.55f), 4.f);

  // 회전하는 룬 조각 (도트 사각형)
  for (int i = 0; i < 6; ++i) {
    const float ang = t * 0.02f + static_cast<float>(i) * kPi / 3.f;
    const float x = std::floor((pos.x + std::cos(ang) * 105.f) / 4.f) * 4.f, y = std::floor((pos.y + std::sin(ang) * 42.f - 10.f) / 4.f) * 4.f;
    g.FillRect(x, y, 8, 8, accent);
    g.FillRect(x + 2, y + 2, 4, 4, Hex(0xffffff));
  }

  // 떠다니는 손 (오른손에는 훔친 모자)
  for (int s = -1; s <= 1; s += 2) {
    const float k = static_cast<float>(s);
    float hx = pos.x + k * (98.f + std::sin(t * 0.05f + k) * 8.f), hy = pos.y + 30.f + std::cos(t * 0.06f + k) * 10.f;
    if (s > 0) {
      const Vec2 h = HatHandPos();
      hx = h.x; hy = h.y;
      if (holdingHat) DrawTreasureHat(g, {hx, hy + 44.f}, 1.f, dying ? 0.8f : 0.3f, t);  // 손끝에 꼭대기만 쥐고 아래로 매달린 모습
    }
    const SpriteId hand = a.bossHand[phase_];
    const Vec2 hs = g.SpriteSize(hand);
    g.DrawSprite(hand, std::floor(hx - hs.x * ps * 0.5f), std::floor(hy - hs.y * ps * 0.5f), ps, s < 0);
  }

  // 본체 (64x64 도트) + 시선을 따라 움직이는 눈동자
  const SpriteId body = a.boss[phase_];
  const Vec2 bs = g.SpriteSize(body);
  const float bx = std::floor(pos.x - bs.x * ps * 0.5f), by = std::floor(pos.y - bs.y * ps * 0.5f);
  g.DrawSprite(body, bx, by, ps);
  const float px = bx + 32.f * ps + std::round(look_.x / 2.f) * 2.f - 3.f, py = by + 31.f * ps + std::round(look_.y / 2.f) * 2.f - 9.f;
  g.FillRect(px, py, 6, 18, Hex(0x12000a));
  if (charge > 0.3f) g.FillRect(px + 2, py + 2, 2, 4, Hex(0xffffff));
  if (flash_ > 0) g.DrawSprite(body, bx, by, ps, false, 0.6f, true);

  if (beam.active) {  // 광선: 도트 블록으로 이어 그린다
    const Vec2 end{pos.x + std::cos(beam.angle) * 1500.f, pos.y + std::sin(beam.angle) * 1500.f};
    if (!beam.firing) {
      g.DashedLine(pos, end, 4.f, 12.f, 12.f, Hex(0xff5078));
    } else {
      g.PixelLine(pos, end, 32.f, Hex(0xb0309a), 4.f);
      g.PixelLine(pos, end, 20.f, Hex(0xff78dc), 4.f);
      g.PixelLine(pos, end, 8.f, Hex(0xffffff), 4.f);
    }
  }
  g.SetGlobalAlpha(1.f);
}

}  // namespace gengar

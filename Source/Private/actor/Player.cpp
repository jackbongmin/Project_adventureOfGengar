#include "actor/Player.h"

#include <algorithm>
#include <cmath>

#include "actor/PhantomArt.h"
#include "actor/boss/Boss.h"
#include "actor/enemy/Enemy.h"
#include "actor/projectile/PlayerSkills.h"
#include "actor/projectile/ShadowBall.h"
#include "controller/PlayerController.h"
#include "controller/BotController.h"
#include "core/DebugOptions.h"
#include "core/Config.h"
#include "core/Random.h"
#include "core/Session.h"
#include "core/Settings.h"
#include "data/PlayerStats.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {
namespace {

constexpr int kDashFrames = 10;     // 약 0.17초 (속도는 stats::DashSpeed, 기본 약 110px)
constexpr int kDashGap = 14;        // 대시 칸이 여러 개여도 이만큼은 떨어져야 다시 대시
constexpr int kGhostLife = 12;
constexpr float kRushReach = 22.f;  // 그림자 돌진: 몸 앞으로 더 베는 거리
const Color kPhaseGreen = Hex(0x6fffa8);  // 무적 대시 중 유령빛
// 차지샷: 오래 모을수록 단계가 오른다. 단계별 모으는 시간(프레임)과 피해·크기 배율
constexpr int kChargeTiers = 4;
constexpr int kChargeFrames[kChargeTiers] = {30, 60, 105, 150};  // 0.5초 / 1초 / 1.75초 / 2.5초
constexpr float kChargeDamage[kChargeTiers] = {2.f, 4.f, 7.f, 10.f};
constexpr float kChargeSize[kChargeTiers] = {1.4f, 1.8f, 2.2f, 2.6f};
const Color kChargeColor[kChargeTiers] = {Hex(0xc58bff), Hex(0x7ae0ff), Hex(0xff7ad9), Hex(0xffe27a)};

}  // namespace

Player::Player(Vec2 p, int maxHp) : Character(ActorKind::Player, p, {22.f, 30.f}, maxHp) {
  if (Debug().bot) SetController(std::make_unique<BotController>());
  else SetController(std::make_unique<PlayerController>());
}

void Player::Update(World& w) {
  if (defeated_) { ++defeatedTimer_; return; }

  intent_ = Intent{};
  controller_->Think(*this, w, intent_);
  const Session& s = w.Sess();
  const bool wasOnGround = onGround_;
  const float fallSpeed = vel.y;
  const Vec2 feet{pos.x + size.x * 0.5f, pos.y + size.y};
  squash_.x += (1.f - squash_.x) * 0.2f;
  squash_.y += (1.f - squash_.y) * 0.2f;
  for (Ghost& g : ghosts_) --g.life;
  ghosts_.erase(std::remove_if(ghosts_.begin(), ghosts_.end(), [](const Ghost& g) { return g.life <= 0; }), ghosts_.end());

  // ---- 대시: 칸 충전 ----
  const int maxCharges = stats::DashCharges(s);
  if (charges_ < 0) charges_ = maxCharges;
  if (charges_ < maxCharges) {
    if (++rechargeTimer_ >= stats::DashRecharge(s)) { ++charges_; rechargeTimer_ = 0; }
  } else {
    charges_ = maxCharges;
    rechargeTimer_ = 0;
  }
  if (onGround_) airDashesUsed_ = 0;
  if (dashGap_ > 0) --dashGap_;
  dashBuffer_ = intent_.dash ? 5 : std::max(0, dashBuffer_ - 1);
  if (dashBuffer_ > 0 && TryStartDash(w)) dashBuffer_ = 0;

  // ---- 수평 이동 (가속/마찰) ----
  const float maxSpeed = stats::MoveSpeed(s);
  if (dashTimer_ > 0) {
    --dashTimer_;
    vel.x = static_cast<float>(dashDir_) * stats::DashSpeed(s);
    vel.y = 0.f;
    const bool phase = s.Has(CardId::PhaseDash);
    if (dashTimer_ % 2 == 0) ghosts_.push_back({{Bounds().Center().x, Bounds().Center().y - 7.f}, facing_, static_cast<float>(animFrame_), kGhostLife, phase});
    if (phase) w.Fx().Burst(Bounds().Center(), 2, kPhaseGreen, 1.5f, 12.f);  // 무적 대시: 초록 유령 불씨
    UpdateDashSkills(w);
    if (dashTimer_ == 0) vel.x = static_cast<float>(dashDir_) * maxSpeed;  // 끝나면 최고 속도로 이어 달린다
  } else if (intent_.moveX != 0.f) {
    vel.x += intent_.moveX * 0.55f;
    SetFacing(Sign(intent_.moveX));
  } else {
    vel.x *= onGround_ ? 0.78f : 0.93f;
  }
  if (dashTimer_ == 0) vel.x = Clamp(vel.x, -maxSpeed, maxSpeed);
  if (dashTimer_ == 0 && intent_.moveX == 0.f && std::fabs(vel.x) < 0.05f) vel.x = 0.f;

  // ---- 점프 (코요테 타임 + 입력 버퍼 + 가변 높이) ----
  jumpBuffer_ = intent_.jumpPressed ? 7 : std::max(0, jumpBuffer_ - 1);
  coyote_ = onGround_ ? 6 : std::max(0, coyote_ - 1);
  if (jumpBuffer_ > 0 && coyote_ > 0) {
    vel.y = -stats::JumpSpeed(s);
    jumpBuffer_ = 0;
    coyote_ = 0;
    onGround_ = false;
    dashTimer_ = 0;  // 대시 중 점프: 대시를 끊고 뛰어오른다
    w.Snd().Play(Sfx::Jump);
    w.Fx().Dust(feet, 6);
    squash_ = {0.75f, 1.3f};  // 위로 쭉 늘어난다
  }
  if (onGround_) airJumpsLeft_ = stats::AirJumps(s);
  // 이단 점프: 공중에서 "이번 프레임에 막 누른" 점프만 쓴다 (착지 직전 버퍼 입력은 땅 점프로 남긴다)
  if (intent_.jumpPressed && jumpBuffer_ > 0 && coyote_ == 0 && !onGround_ && airJumpsLeft_ > 0) {
    --airJumpsLeft_;
    vel.y = -stats::JumpSpeed(s) * 0.92f;
    jumpBuffer_ = 0;
    dashTimer_ = 0;
    squash_ = {0.7f, 1.35f};
    const Vec2 c{pos.x + size.x * 0.5f, pos.y + size.y};
    w.Fx().Burst(c, 10, Hex(0xd9b8ff), 2.5f, 18.f);  // 발밑에서 보랏빛이 팡
    w.Snd().Tone(520.f, 1040.f, 0.1f, Wave::Square, 0.05f);
  }
  // 가변 점프: 점프 키를 일찍 떼면 상승을 깎는다. 설정에 따라 고정 높이 / 보통 / 더 낮게.
  if (Prefs().jump != JumpControl::Fixed && !intent_.jumpHeld && vel.y < -3.5f) vel.y *= Prefs().jump == JumpControl::ShortTap ? 0.6f : 0.8f;

  if (dashTimer_ == 0) {  // 대시 중에는 수평으로 곧게
    const bool floating = s.Has(CardId::Float) && !onGround_ && intent_.jumpHeld && vel.y > 0.f;  // 유령 부유: 천천히 떨어진다
    if (floating) vel.y = std::min(vel.y + cfg::kGravity * 0.35f, cfg::kMaxFall * 0.25f);
    else vel.y = std::min(vel.y + cfg::kGravity, cfg::kMaxFall);
  }
  MoveAndCollide(w);
  if (pos.y > static_cast<float>(cfg::kRows * cfg::kTile) + 40.f) { Defeat(w); return; }

  // ---- 착지/달리기 먼지 + 착지 찌그러짐 ----
  const Vec2 newFeet{pos.x + size.x * 0.5f, pos.y + size.y};
  if (!wasOnGround && onGround_ && fallSpeed > 3.f) {
    const float k = std::min(1.f, fallSpeed / cfg::kMaxFall);
    squash_ = {1.f + 0.4f * k, 1.f - 0.35f * k};
    const int n = 2 + static_cast<int>(k * 5.f);
    w.Fx().Dust(newFeet, n, -1.f);
    w.Fx().Dust(newFeet, n, 1.f);
  } else if (onGround_) {
    const bool skid = intent_.moveX != 0.f && Sign(intent_.moveX) != Sign(vel.x) && std::fabs(vel.x) > 2.f;
    if (skid && animFrame_ % 3 == 0) w.Fx().Dust(newFeet, 1, Sign(vel.x) * 0.8f);  // 급정거
    else if (std::fabs(vel.x) > maxSpeed * 0.75f && animFrame_ % 8 == 0) w.Fx().Dust(newFeet, 1, -static_cast<float>(facing_) * 0.6f);
  }

  // ---- 공격 ----
  if (cooldown_ > 0) --cooldown_;
  if (s.Has(CardId::ChargeShot)) {  // 차지샷: 누른 순간 한 발, 꾹 누르고 있으면 단계가 오르고, 떼면 그 단계의 큰 공
    if (intent_.shoot) {
      if (!shootHeld_ && cooldown_ <= 0) Shoot(w, 0);
      const int before = ChargeTier();
      charge_ = std::min(charge_ + 1, kChargeFrames[kChargeTiers - 1]);
      if (const int tier = ChargeTier(); tier > before) {  // 단계가 오를 때마다 높아지는 소리 + 그 단계 색 불꽃
        w.Snd().Tone(440.f + 220.f * static_cast<float>(tier), 880.f + 440.f * static_cast<float>(tier), 0.1f, Wave::Square, 0.05f);
        w.Fx().Burst(Bounds().Center(), 6 + 3 * tier, kChargeColor[tier - 1], 2.f + static_cast<float>(tier), 14.f);
      }
    } else {
      if (shootHeld_ && ChargeTier() > 0) Shoot(w, ChargeTier());
      charge_ = 0;
    }
    shootHeld_ = intent_.shoot;
  } else if (intent_.shoot && cooldown_ <= 0) {
    Shoot(w, 0);
  }
  if ((intent_.ultimate || ultQueued_) && s.gauge >= 100.f) CastUltimate(w);
  ultQueued_ = false;
  UpdateRain(w);

  if (invuln_ > 0) --invuln_;
  ++animFrame_;
  CheckHazards(w);
}

void Player::LatchInput(World& w) {
  if (defeated_ || !controller_) return;
  Intent in;
  controller_->Think(*this, w, in);
  if (in.jumpPressed) jumpBuffer_ = 8;  // Update에서 1 줄어든 뒤 평소 버퍼(7)와 같아진다
  if (in.ultimate) ultQueued_ = true;
  if (in.dash) dashBuffer_ = 6;
}

bool Player::TryStartDash(World& w) {
  const Session& s = w.Sess();
  if (dashTimer_ > 0 || dashGap_ > 0 || charges_ <= 0 || (!onGround_ && airDashesUsed_ >= stats::AirDashes(s))) return false;
  dashDir_ = intent_.moveX != 0.f ? Sign(intent_.moveX) : facing_;
  SetFacing(dashDir_);
  dashTimer_ = kDashFrames;
  dashGap_ = kDashGap;
  --charges_;
  if (!onGround_) ++airDashesUsed_;
  DebugLog("[player] dash (frame %d, charges left %d)", w.Frame(), charges_);
  rushHits_.clear();
  if (s.Has(CardId::Afterimage)) w.SpawnPooled<DashBlast>(Vec2{Bounds().Center().x, Bounds().Center().y - 7.f}, facing_, stats::Damage(s) * 3.f, 56.f);
  squash_ = {1.4f, 0.75f};  // 옆으로 쭉 늘어난다
  const Vec2 feet{pos.x + size.x * 0.5f, pos.y + size.y};
  if (onGround_) w.Fx().Dust(feet, 6, -static_cast<float>(dashDir_) * 1.5f);
  else w.Fx().Burst(Bounds().Center(), 6, Hex(0xc58bff), 2.f, 14.f);
  w.Snd().Tone(260.f, 820.f, 0.12f, Wave::Saw, 0.06f);
  return true;
}

bool Player::LastStand(const World& w) const { return hp_ == 1 && w.Sess().Has(CardId::LastStand); }

ShotSpec Player::MakeShot(const World& w) const {
  const Session& s = w.Sess();
  ShotSpec sp;
  sp.radius = stats::ShotRadius(s);
  sp.damage = stats::Damage(s) * (LastStand(w) ? 1.5f : 1.f);
  sp.life = static_cast<int>(static_cast<float>(stats::ShotLife(s)) * stats::BounceLifeScale(s));
  sp.pierce = stats::Pierce(s);
  sp.bounces = stats::Bounces(s);
  sp.homing = s.Has(CardId::Homing);
  sp.burst = s.Has(CardId::BurstShot);
  sp.split = s.Has(CardId::Split);
  sp.curse = s.Has(CardId::Curse);
  sp.crit = stats::CritChance(s);
  sp.knockback = stats::Knockback(s);
  return sp;
}

void Player::Shoot(World& w, int chargeTier) {
  const Session& s = w.Sess();
  const ShotSpec base = MakeShot(w);
  const float speed = stats::ShotSpeed(s);
  const Vec2 origin{pos.x + size.x * 0.5f + static_cast<float>(facing_) * 14.f, pos.y + 15.f};
  if (chargeTier > 0) {  // 차지샷: 단계에 따라 피해 2/4/7/10배, 모든 적을 관통
    const int k = chargeTier - 1;
    ShotSpec big = base;
    big.radius *= kChargeSize[k];
    big.damage *= kChargeDamage[k];
    big.life = static_cast<int>(static_cast<float>(base.life) * 1.3f);
    big.pierce = 99;
    big.charged = true;
    w.SpawnPooled<ShadowBall>(origin, Vec2{static_cast<float>(facing_) * speed * 0.85f, 0.f}, big);
    w.Fx().Shake(2.f + 2.f * static_cast<float>(chargeTier));
    w.Fx().Burst(origin, 8 + 6 * chargeTier, kChargeColor[k], 2.f + static_cast<float>(chargeTier), 18.f);
    w.Snd().Tone(260.f - 30.f * static_cast<float>(chargeTier), 110.f, 0.12f + 0.04f * static_cast<float>(chargeTier), Wave::Saw, 0.08f);
    cooldown_ = stats::FireCooldown(s);
    return;
  }
  const int n = std::min(3, stats::ShotCount(s));
  const float spreads[3][3] = {{0.f, 0.f, 0.f}, {-0.7f, 0.7f, 0.f}, {-1.5f, 0.f, 1.5f}};
  for (int i = 0; i < n; ++i) {
    w.SpawnPooled<ShadowBall>(origin, Vec2{static_cast<float>(facing_) * speed, spreads[n - 1][i]}, base);
  }
  if (s.Has(CardId::BackShot)) {  // 후방 사격: 뒤로도 같은 수만큼, 피해 50%
    ShotSpec back = base;
    back.damage *= 0.5f;
    const Vec2 behind{pos.x + size.x * 0.5f - static_cast<float>(facing_) * 14.f, pos.y + 15.f};
    for (int i = 0; i < n; ++i) {
      w.SpawnPooled<ShadowBall>(behind, Vec2{-static_cast<float>(facing_) * speed, spreads[n - 1][i]}, back);
    }
  }
  w.Fx().Burst(origin, 5, Hex(0xc58bff), 2.2f, 10.f);  // 손끝에서 튀는 불꽃 (발사 임팩트)
  cooldown_ = stats::FireCooldown(s);
  if (LastStand(w)) cooldown_ = static_cast<int>(static_cast<float>(cooldown_) / 1.3f);
  w.Snd().Play(Sfx::Shoot);
}

// 그림자 돌진: 대시로 통과한 적을 한 번씩 벤다. 앞에 그리는 칼날 연출만큼 판정도 앞으로 조금 넓힌다.
void Player::UpdateDashSkills(World& w) {
  const Session& s = w.Sess();
  if (!s.Has(CardId::ShadowRush)) return;
  DamageInfo info{stats::Damage(s) * 2.f, dashDir_};
  info.fromPlayer = true;
  info.knockback = 3.f;
  Rect box = Bounds();
  if (dashDir_ < 0) box.x -= kRushReach;
  box.w += kRushReach;
  auto slash = [&](Actor& a, bool boss) {
    if (std::find(rushHits_.begin(), rushHits_.end(), &a) != rushHits_.end() || !a.Bounds().Overlaps(box)) return;
    rushHits_.push_back(&a);
    if (boss) static_cast<Boss&>(a).TakeDamage(w, info);
    else static_cast<Enemy&>(a).TakeDamage(w, info);
    w.Fx().Burst(a.Bounds().Center(), 10, Hex(0xe6ccff), 4.f, 14.f);
  };
  w.ForEach(ActorKind::Enemy, [&](Actor& a) { slash(a, false); });
  if (Boss* boss = w.GetBoss(); boss && !boss->dying && !boss->invulnerable) slash(*boss, true);
}

// 필살기: 장착한 종류에 따라 다르다. 공통으로 게이지를 비우고 잠시 무적.
void Player::CastUltimate(World& w) {
  Session& s = w.Sess();
  DebugLog("[player] ultimate cast (type %d)", static_cast<int>(s.prog.ult));
  s.gauge = 0.f;
  const float mul = stats::UltMultiplier(s);
  const Vec2 c = Bounds().Center();
  switch (s.prog.ult) {
    case UltType::Pulse: {  // 섀도우 펄스: 화면 안의 모든 적에게 큰 피해, 적 탄환 제거
      w.SetUltActive(true);
      DamageInfo info{stats::UltDamage(s), 1, false, true};
      info.fromPlayer = true;
      const Rect view{w.Cam().x - 40.f, 0.f, static_cast<float>(cfg::kScreenW) + 80.f, static_cast<float>(cfg::kScreenH)};
      w.ForEach(ActorKind::Enemy, [&](Actor& a) {
        if (a.Bounds().Overlaps(view)) static_cast<Enemy&>(a).TakeDamage(w, info);
      });
      if (Boss* boss = w.GetBoss()) { info.amount = stats::UltBossDamage(s); boss->TakeDamage(w, info); }
      w.SetUltActive(false);
      w.ForEach(ActorKind::EnemyShot, [](Actor& a) { a.Kill(); });
      invuln_ = std::max(invuln_, 70);
      w.Fx().AddRing(c);
      break;
    }
    case UltType::Rain:  // 섀도우 레인: 3초 동안 화면 위에서 그림자 탄이 적에게 떨어진다
      rainTimer_ = 180;
      invuln_ = std::max(invuln_, 70);
      break;
    case UltType::BlackHole:  // 블랙홀: 앞쪽에 4초 동안
      w.Spawn<BlackHole>(Vec2{c.x + static_cast<float>(facing_) * 200.f, c.y - 20.f}, 1.5f * mul, 3.f * mul);
      invuln_ = std::max(invuln_, 70);
      break;
    case UltType::Nightmare:  // 나이트메어: 4초 동안 시간 정지 + 무적, 끝나면 화면 전체 피해
      w.StartTimeStop(240);
      invuln_ = std::max(invuln_, 250);
      break;
  }
  w.Fx().Shake(16.f);
  w.Fx().Flash();
  w.Fx().Burst(c, 60, Hex(0xc58bff), 9.f, 40.f);
  w.Snd().Play(Sfx::Ultimate);
}

void Player::UpdateRain(World& w) {
  if (rainTimer_ <= 0) return;
  --rainTimer_;
  if (rainTimer_ % 10 != 0) return;  // 3초 동안 18발
  const Session& s = w.Sess();
  const float viewL = w.Cam().x, viewR = viewL + static_cast<float>(cfg::kScreenW);
  std::vector<Vec2> targets;
  w.ForEach(ActorKind::Enemy, [&](Actor& a) {
    const Vec2 c = a.Bounds().Center();
    if (c.x > viewL && c.x < viewR) targets.push_back(c);
  });
  if (Boss* boss = w.GetBoss(); boss && !boss->dying) targets.push_back(boss->pos);
  const Vec2 to = targets.empty() ? Vec2{RandRange(viewL + 60.f, viewR - 60.f), 420.f}
                                  : targets[static_cast<size_t>(RandInt(0, static_cast<int>(targets.size()) - 1))];
  const Vec2 from{to.x + RandRange(-80.f, 80.f), 72.f};  // 동굴 천장(0~1행) 아래에서 떨어진다
  ShotSpec rain;
  rain.radius = 9.f;
  rain.damage = 3.f * stats::UltMultiplier(s);
  rain.bossScale = 2.f / 3.f;  // 보스에게는 2
  rain.life = 90;
  rain.ultimate = true;
  const float d = std::max(1.f, std::hypot(to.x - from.x, to.y - from.y));
  w.SpawnPooled<ShadowBall>(from, Vec2{(to.x - from.x) / d * 9.f, (to.y - from.y) / d * 9.f}, rain);
}

float Player::DashRefill(const World& w) const {
  return static_cast<float>(rechargeTimer_) / static_cast<float>(std::max(1, stats::DashRecharge(w.Sess())));
}

float Player::ChargeRatio() const { return std::min(1.f, static_cast<float>(charge_) / static_cast<float>(kChargeFrames[kChargeTiers - 1])); }
int Player::ChargeTier() const {
  int tier = 0;
  while (tier < kChargeTiers && charge_ >= kChargeFrames[tier]) ++tier;
  return tier;
}

void Player::CheckHazards(World& w) {
  const Level& lv = w.GetLevel();
  const float T = static_cast<float>(cfg::kTile);
  const int c0 = static_cast<int>(std::floor((pos.x + 3.f) / T)), c1 = static_cast<int>(std::floor((pos.x + size.x - 3.f) / T));
  const int r = static_cast<int>(std::floor((pos.y + size.y - 1.f) / T));
  for (int c = c0; c <= c1; ++c) {
    if (lv.At(c, r) == Tile::Spike && pos.y + size.y > static_cast<float>(r) * T + 14.f) {
      TakeDamage(w, DamageInfo{1.f, -facing_, false, false, "spike"});
      break;
    }
  }
  if (lv.HasFlag() && Bounds().Overlaps(lv.Flag()) && !w.StageCleared()) {
    Session& s = w.Sess();
    s.score += 500 + hp_ * 100;
    w.NotifyStageCleared();
    w.Snd().Play(Sfx::StageClear);
    w.Fx().Burst(Bounds().Center(), 40, Hex(0xffe27a), 6.f, 60.f);
  }
}

void Player::TakeDamage(World& w, const DamageInfo& info) {
  if (invuln_ > 0 || defeated_) return;
  if (dashTimer_ > 0 && w.Sess().Has(CardId::PhaseDash)) return;  // 무적 대시로 회피 (기본 대시는 맞는다)
  if (Debug().god) {  // 검증용 무적: 피격은 무시하되 출처를 기록한다
    invuln_ = 20;
    DebugLog("[player] hit by %s (ignored: god)", info.source);
    return;
  }
  if (shield_) {  // 그림자 보호막이 대신 맞는다
    shield_ = false;
    invuln_ = 60;
    w.HitStop(4);
    w.Fx().Pop({Bounds().Center().x, pos.y - 10.f}, "BLOCK", Hex(0x7ae0ff));
    w.Fx().Burst(Bounds().Center(), 16, Hex(0x7ae0ff), 4.f, 20.f);
    w.Snd().Play(Sfx::BossBlink);
    return;
  }
  --hp_;
  invuln_ = stats::HurtInvuln(w.Sess());
  w.HitStop(6);
  w.BreakCombo();
  squash_ = {1.25f, 0.8f};
  vel.y = -6.f;
  vel.x = static_cast<float>(info.dir) * 5.f;
  w.Fx().Shake(8.f);
  w.Snd().Play(Sfx::PlayerHurt);
  w.Fx().Burst(Bounds().Center(), 12, Hex(0xc58bff), 4.f);
  if (hp_ <= 0) Defeat(w);
}

// 그림자 돌진 연출: 몸 앞에 초승달 모양 칼날이 위에서 아래로 휘두르듯 펼쳐진다.
// 도트 단위(3px)로 호를 찍고, 가운데는 흰색, 바깥은 보라. 대시가 끝나 갈수록 옅어진다.
void Player::DrawRushSlash(Graphics& gfx, Vec2 c) const {
  constexpr float kPx = 3.f;
  const float k = 1.f - static_cast<float>(dashTimer_) / static_cast<float>(kDashFrames);  // 0 -> 1 (대시 진행)
  const float sweep = std::min(1.f, k * 3.f + 0.4f);  // 호가 위에서 아래로 그려지는 정도 (대시 초반에 다 펼쳐진다)
  const float alpha = dashTimer_ > 3 ? 1.f : static_cast<float>(dashTimer_) / 3.f;
  const float dir = static_cast<float>(dashDir_);
  const Vec2 o{c.x + dir * 6.f, c.y - 6.f};  // 호의 중심 (몸 살짝 앞)
  constexpr float kR = 34.f, kHalf = 1.2f;   // 반지름, 위아래로 펼쳐지는 각도(rad)
  const float a0 = -kHalf, a1 = -kHalf + 2.f * kHalf * sweep;
  for (float a = a0; a <= a1; a += 0.09f) {
    const float edge = 1.f - std::fabs(a) / kHalf;  // 가운데가 두껍고 끝이 가늘다
    const float thick = 6.f + 12.f * edge;
    for (float d = 0.f; d < thick; d += kPx) {
      const float r = kR + kRushReach * 0.4f - d;
      const float x = std::floor((o.x + dir * std::cos(a) * r) / kPx) * kPx, y = std::floor((o.y + std::sin(a) * r) / kPx) * kPx;
      const Color col = d < kPx ? Rgb(255, 255, 255, static_cast<int>(240.f * alpha)) : d < kPx * 2.f ? Rgb(230, 204, 255, static_cast<int>(220.f * alpha))
                                                                                           : Rgb(160, 90, 255, static_cast<int>(170.f * alpha));
      gfx.FillRect(x, y, kPx, kPx, col);
    }
  }
  gfx.Glow(o.x + dir * kR, o.y, 26.f, Rgb(200, 150, 255, static_cast<int>(90.f * alpha)));
}

void Player::Defeat(World& w) {
  if (defeated_) return;
  defeated_ = true;
  defeatedTimer_ = 0;
  w.BreakCombo();
  w.Fx().Shake(12.f);
  w.Fx().Burst(Bounds().Center(), 40, Hex(0xb36bff), 6.f, 50.f);
  w.Snd().Play(Sfx::PlayerDie);
}

void Player::Render(Graphics& gfx, const World& w) {
  if (defeated_) return;
  for (const Ghost& g : ghosts_) {  // 대시 잔상: 오래된 것일수록 옅게. 무적 대시 잔상은 초록빛을 두른다
    const float k = static_cast<float>(g.life) / static_cast<float>(kGhostLife);
    if (g.phase) gfx.Glow(g.center.x, g.center.y, 26.f, WithAlpha(kPhaseGreen, 0.35f * k));
    DrawPhantom(gfx, g.center, g.facing, g.time, 1.f, (g.phase ? 0.35f : 0.45f) * k);
  }
  if (invuln_ > 0 && (invuln_ / 4) % 2) return;  // 무적 깜빡임
  const Vec2 c = Bounds().Center();
  PhantomStyle style;
  style.happy = style.hat = wearingHat_;
  style.hatGlow = wearingHat_ ? 1.f : 0.f;
  style.squash = squash_;
  if (charge_ > 10) {  // 차지샷 모으는 중: 단계마다 손끝 빛의 색이 바뀌고, 최대 단계면 금빛으로 깜빡인다
    const float k = ChargeRatio();
    const int tier = ChargeTier();
    const bool full = tier == kChargeTiers;
    const Color glow = tier > 0 ? kChargeColor[tier - 1] : Hex(0xc58bff);
    gfx.Glow(c.x + static_cast<float>(facing_) * 14.f, c.y - 2.f, 10.f + 22.f * k, full && (animFrame_ / 4) % 2 ? Rgb(255, 255, 255, 170) : WithAlpha(glow, 0.5f));
    for (int i = 0; i < tier; ++i) {  // 머리 위 작은 점 = 모은 단계
      gfx.FillRect(c.x - 13.f + 7.f * static_cast<float>(i), pos.y - 22.f, 5, 5, kChargeColor[i]);
    }
  }
  // 무적 대시 중: 반투명해지고 초록 유령빛이 감싼다 (맞지 않는 상태라는 표시)
  const bool phasing = dashTimer_ > 0 && w.Sess().Has(CardId::PhaseDash);
  if (phasing) {
    gfx.Glow(c.x, c.y - 7.f, 34.f, WithAlpha(kPhaseGreen, 0.55f));
    gfx.PixelRing(c.x, c.y - 7.f, 22.f, 2.f, WithAlpha(kPhaseGreen, 0.6f), 2.f);
  }
  DrawPhantom(gfx, {c.x, c.y - 7.f}, facing_, static_cast<float>(animFrame_), 1.f, phasing ? 0.5f : 1.f, style);  // 스프라이트 발끝을 히트박스 바닥에 맞춘다
  if (dashTimer_ > 0 && w.Sess().Has(CardId::ShadowRush)) DrawRushSlash(gfx, c);
  if (shield_) gfx.PixelRing(c.x, c.y - 4.f, 26.f + std::sin(static_cast<float>(animFrame_) * 0.1f) * 2.f, 2.f, Rgb(122, 224, 255, 140), 2.f);
}

}  // namespace gengar

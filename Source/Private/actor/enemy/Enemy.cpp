#include "actor/enemy/Enemy.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <typeinfo>

#include "actor/Player.h"
#include "actor/pickup/Pickups.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "core/Random.h"
#include "core/Session.h"
#include "data/PlayerStats.h"
#include "platform/Graphics.h"
#include "platform/Audio.h"
#include "world/World.h"

namespace gengar {
namespace {

constexpr float kStompSlack = 6.f;       // 직전 프레임 발이 머리보다 이만큼 아래여도 밟기로 인정 (모서리 판정 여유)
constexpr int kStompGraceFrames = 12;    // 밟은 뒤 튕겨 나가 떨어질 때까지 몸통 피해를 주지 않는 시간

}  // namespace

Enemy::Enemy(Vec2 p, Vec2 s, int hp) : Character(ActorKind::Enemy, p, s, hp) {}

void Enemy::MakeElite() {
  elite_ = true;
  maxHp_ = std::max(maxHp_ * 3, maxHp_ + 40);
  hp_ = maxHp_;
}

void Enemy::Update(World& w) {
  if (flash_ > 0) --flash_;
  if (stompGrace_ > 0) --stompGrace_;
  if (curse_ > 0) --curse_;
  if (curseCooldown_ > 0) --curseCooldown_;
  intent_ = Intent{};
  if (controller_) controller_->Think(*this, w, intent_);
  ApplyIntent(w);
  if (slow_ > 0) {  // 냉기: 걷는 속도(박쥐는 나는 속도까지) 절반
    --slow_;
    vel.x *= 0.5f;
    if (Kind() == ActorKind::Enemy && intent_.flyVel.y != 0.f) vel.y *= 0.5f;
    if (slow_ % 10 == 0) w.Fx().Burst(Bounds().Center(), 1, Hex(0x9ff0ff), 1.f, 12.f);
  }
  if (poison_ > 0 && --poison_ % 30 == 0) {  // 독: 0.5초마다
    DamageInfo tick{poisonDmg_, facing_};
    tick.dot = true;
    w.Fx().Burst(Bounds().Center(), 3, Hex(0x6fe08a), 1.5f, 14.f);
    TakeDamage(w, tick);
    if (!IsAlive()) return;
  }
  if (std::fabs(knockVel_) > 0.05f) {
    vel.x += knockVel_;
    knockVel_ *= 0.8f;
  }
  StepPhysics(w);
  if (intent_.shoot) OnShoot(w, intent_.aim);
  if (pos.y > static_cast<float>(cfg::kScreenH) + 100.f) { DebugLog("[enemy] %s fell into a pit at x=%d", typeid(*this).name(), static_cast<int>(pos.x)); Kill(); return; }
  if (elite_) {
    if (warn_ > 0) --warn_;
    UpdateEliteSkill(w);
  }
  ContactPlayer(w);
}

void Enemy::ApplyIntent(World&) {
  vel.x = intent_.moveX * moveSpeed_;
  if (intent_.moveX != 0.f) SetFacing(Sign(intent_.moveX));  // 순찰처럼 이동 값이 소수여도 방향이 갱신되게 부호로 변환
  if (intent_.jumpPressed && onGround_) vel.y = -jumpSpeed_;
}

void Enemy::StepPhysics(World& w) {
  vel.y = std::min(vel.y + cfg::kGravity, 10.f);
  MoveAndCollide(w);
}

void Enemy::OnShoot(World&, Vec2) {}

void Enemy::ContactPlayer(World& w) {
  Player& p = w.GetPlayer();
  if (!p.IsAlive() || !Bounds().Overlaps(p.Bounds())) return;
  if (stompGrace_ > 0) {  // 방금 밟혀서 플레이어가 튕겨 오르는 중. 떨어질 때까지 유예를 유지한다 (적이 따라 뛰어오르는 경우)
    stompGrace_ = std::max(stompGrace_, 3);
    return;
  }
  // 위에서 떨어지며 밟으면 적이 피해를 입고 플레이어는 튕긴다.
  // 겹친 깊이가 아니라 "직전 프레임에 플레이어 발이 적 머리보다 위에 있었고, 위에서 다가왔는가"로 판정한다.
  // (빠르게 떨어지거나 적이 위로 뛰어오르면 한 프레임에 18px 넘게 파고들 수 있다)
  const float prevFeet = p.Bounds().Bottom() - p.vel.y;
  const float prevHead = pos.y - vel.y;
  if (p.vel.y > vel.y && prevFeet <= prevHead + kStompSlack) {
    DebugLog("[enemy] stomped (depth %.1f, player vy %.1f, enemy vy %.1f, hp %d -> %d)", p.Bounds().Bottom() - pos.y, p.vel.y, vel.y, hp_, hp_ - 2);
    stompGrace_ = kStompGraceFrames;
    p.Bounce(-8.5f);
    TakeDamage(w, DamageInfo{20.f, 1, true, false});
  } else {
    DebugLog("[enemy] body contact (depth %.1f, player vy %.1f)", p.Bounds().Bottom() - pos.y, p.vel.y);
    const bool couldHurt = !p.Invulnerable();
    const int dir = Sign(p.pos.x - pos.x) != 0 ? Sign(p.pos.x - pos.x) : 1;
    p.TakeDamage(w, DamageInfo{1.f, dir, false, false, "enemy-contact"});
    if (couldHurt && w.Sess().Has(CardId::Thorns)) {  // 가시 반격: 부딪힌 적에게 30
      DamageInfo thorns{30.f, -dir};
      thorns.fromPlayer = true;
      thorns.knockback = 6.f;
      TakeDamage(w, thorns);
    }
  }
}

void Enemy::TakeDamage(World& w, const DamageInfo& info) {
  if (!IsAlive()) return;
  float amount = info.amount;
  if (info.fromPlayer) amount *= stats::TargetMultiplier(w.Sess(), static_cast<float>(hp_) / static_cast<float>(maxHp_), curse_ > 0);
  const int dmg = std::max(1, static_cast<int>(std::lround(amount)));
  if (!info.ultimate && !info.dot) w.AddGauge(hp_ - dmg <= 0 ? 15.f : 4.f);
  hp_ -= dmg;
  flash_ = 6;
  if (info.curse && curseCooldown_ == 0) { curse_ = stats::kCurseFrames; curseCooldown_ = stats::kCurseCooldown; }
  if (info.poison && !info.dot) {  // 독: 이미 걸려 있으면 더 센 쪽으로, 3초 다시 채운다
    if (poison_ <= 0) poisonDmg_ = 0.f;
    poison_ = 180;
    poisonDmg_ = std::max(poisonDmg_, info.amount * 0.25f);
  }
  if (info.frost) slow_ = 120;
  if (info.knockback > 0.f) Push(static_cast<float>(info.dir) * info.knockback);
  const Vec2 c = Bounds().Center();
  if (info.dot) w.Fx().Pop({c.x, pos.y}, "-" + std::to_string(dmg), Hex(0x6fe08a));
  else if (info.crit) w.Fx().Pop({c.x, pos.y - 6.f}, "-" + std::to_string(dmg) + "!", Hex(0xffe27a));
  else w.Fx().Pop({c.x, pos.y}, "-" + std::to_string(dmg));
  if (!info.dot) w.Snd().Play(Sfx::EnemyHit);
  if (hp_ > 0) { if (!info.dot) w.HitStop(2); return; }

  w.HitStop(5);
  w.Fx().Shake(3.f);
  const int mult = w.RegisterKill(c);
  const int coins = elite_ ? 12 : 3;
  w.AddScore((elite_ ? 500 : 100) * mult);
  w.AddCoins(coins);
  w.AddXp(XpValue() * (elite_ ? 4 : 1));
  w.OnEnemyKilled();
  w.Fx().Burst(c, elite_ ? 36 : 18, DeathColor(), elite_ ? 6.f : 4.f);
  if (elite_) { w.Fx().Burst(c, 20, Hex(0xffe27a), 5.f, 30.f); w.Fx().Shake(7.f); w.HitStop(9); }
  w.Fx().Pop({c.x, pos.y - 12.f}, "+" + std::to_string(coins), Hex(0xffe27a));
  if (Chance(elite_ ? 0.5f : 0.12f)) w.Spawn<Heart>(c);
  w.Snd().Play(Sfx::EnemyDie);
  OnDeath(w);
  Kill();
}

}  // namespace gengar

#include "actor/enemy/Enemy.h"

#include <cmath>
#include <string>

#include "actor/Player.h"
#include "actor/pickup/Pickups.h"
#include "core/Config.h"
#include "core/Random.h"
#include "platform/Audio.h"
#include "world/World.h"

namespace gengar {

Enemy::Enemy(Vec2 p, Vec2 s, int hp) : Character(ActorKind::Enemy, p, s, hp) {}

void Enemy::Update(World& w) {
  if (flash_ > 0) --flash_;
  intent_ = Intent{};
  if (controller_) controller_->Think(*this, w, intent_);
  ApplyIntent(w);
  StepPhysics(w);
  if (intent_.shoot) OnShoot(w, intent_.aim);
  if (pos.y > static_cast<float>(cfg::kScreenH) + 100.f) { Kill(); return; }
  ContactPlayer(w);
}

void Enemy::ApplyIntent(World&) {
  vel.x = intent_.moveX * moveSpeed_;
  if (intent_.moveX != 0.f) SetFacing(static_cast<int>(intent_.moveX));
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
  // 위에서 떨어지며 밟으면 적이 피해를 입고 플레이어는 튕긴다.
  if (p.vel.y > 1.f && p.Bounds().Bottom() - pos.y < 18.f) {
    TakeDamage(w, DamageInfo{2.f, 1, true, false});
    p.Bounce(-8.5f);
  } else {
    p.TakeDamage(w, DamageInfo{1.f, Sign(p.pos.x - pos.x) != 0 ? Sign(p.pos.x - pos.x) : 1});
  }
}

void Enemy::TakeDamage(World& w, const DamageInfo& info) {
  if (!IsAlive()) return;
  const int dmg = static_cast<int>(std::lround(info.amount));
  w.AddGauge(hp_ - dmg <= 0 ? 15.f : 4.f);
  hp_ -= dmg;
  flash_ = 6;
  const Vec2 c = Bounds().Center();
  w.Fx().Pop({c.x, pos.y}, "-" + std::to_string(dmg));
  w.Snd().Play(Sfx::EnemyHit);
  if (hp_ > 0) return;

  w.AddScore(100);
  w.AddCoins(3);
  w.Fx().Burst(c, 18, DeathColor(), 4.f);
  w.Fx().Pop({c.x, pos.y - 12.f}, "+3", Hex(0xffe27a));
  if (Chance(0.12f)) w.Spawn<Heart>(c);
  w.Snd().Play(Sfx::EnemyDie);
  Kill();
}

}  // namespace gengar

#pragma once
// 플레이어(보라색 유령). 입력은 PlayerController가 Intent로 변환해 전달한다.
#include "actor/Character.h"

namespace gengar {

class Player : public Character {
 public:
  Player(Vec2 pos, int maxHp);

  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;
  void TakeDamage(World& world, const DamageInfo& info) override;

  bool IsAlive() const override { return !defeated_; }
  int DefeatedTimer() const { return defeatedTimer_; }
  bool Invulnerable() const { return invuln_ > 0; }
  void Heal(int n) { hp_ = std::min(maxHp_, hp_ + n); }
  void Bounce(float vy) { vel.y = vy; onGround_ = false; }  // 적을 밟은 반동

 private:
  void Defeat(World& world);
  void Shoot(World& world);
  void CastUltimate(World& world);
  void CheckHazards(World& world);

  Intent intent_;
  int coyote_ = 0;
  int jumpBuffer_ = 0;
  int invuln_ = 0;
  int cooldown_ = 0;
  int animFrame_ = 0;
  bool defeated_ = false;
  int defeatedTimer_ = 0;
};

}  // namespace gengar

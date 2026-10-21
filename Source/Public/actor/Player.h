#pragma once
// 플레이어(보라색 유령). 입력은 PlayerController가 Intent로 변환해 전달한다.
#include <vector>

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
  bool Dashing() const { return dashTimer_ > 0; }
  void Heal(int n) { hp_ = std::min(maxHp_, hp_ + n); }
  void SetHp(int hp) { hp_ = Clamp(hp, 1, maxHp_); }  // 디버그용
  void Bounce(float vy) { vel.y = vy; onGround_ = false; }  // 적을 밟은 반동
  void SetWearingHat(bool on) { wearingHat_ = on; }  // 보물 모자를 되찾은 뒤 (모자 + 행복한 표정)
  void LatchInput(World& world);  // 히트스톱 중 눌린 점프/필살기를 기억해 두었다가 풀리면 처리한다

 private:
  void Defeat(World& world);
  void Shoot(World& world);
  void CastUltimate(World& world);
  void CheckHazards(World& world);
  bool TryStartDash(World& world);  // 대시 시작 조건(쿨다운/공중 1회)을 만족하면 시작

  Intent intent_;
  int coyote_ = 0;
  int jumpBuffer_ = 0;
  int invuln_ = 0;
  int cooldown_ = 0;
  int animFrame_ = 0;
  bool defeated_ = false;
  int defeatedTimer_ = 0;
  bool wearingHat_ = false;
  bool ultQueued_ = false;
  Vec2 squash_{1.f, 1.f};  // 스쿼시/스트레치 배율 (가로, 세로). 매 프레임 1로 돌아간다.

  // ---- 대시: 짧게 수평으로 돌진, 그동안 피해 무시. 공중에서는 착지 전까지 1번. ----
  int dashTimer_ = 0;     // 남은 대시 프레임
  int dashCooldown_ = 0;
  int dashBuffer_ = 0;    // 쿨다운 직전에 누른 입력도 받아 준다
  int dashDir_ = 1;
  bool airDashUsed_ = false;
  struct Ghost { Vec2 center; int facing; float time; int life; };
  std::vector<Ghost> ghosts_;  // 대시 잔상
};

}  // namespace gengar

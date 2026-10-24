#pragma once
// 플레이어(보라색 유령). 입력은 PlayerController가 Intent로 변환해 전달한다.
// 능력치와 카드 능력은 매 프레임 Session 의 카드 단계(data/PlayerStats.h)에서 읽는다.
#include <vector>

#include "actor/Character.h"
#include "actor/projectile/ShadowBall.h"

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
  void SetMaxHp(int maxHp, int heal) { maxHp_ = maxHp; Heal(heal); }  // 최대 하트 카드
  void GiveShield() { shield_ = true; }
  bool HasShield() const { return shield_; }
  void Bounce(float vy) { vel.y = vy; onGround_ = false; }  // 적을 밟은 반동
  void SetWearingHat(bool on) { wearingHat_ = on; }  // 보물 모자를 되찾은 뒤 (모자 + 행복한 표정)
  void LatchInput(World& world);  // 히트스톱 중 눌린 점프/필살기를 기억해 두었다가 풀리면 처리한다

  // 지금 카드 능력이 모두 반영된 섀도우볼 한 발 (분신도 이걸 쓴다)
  ShotSpec MakeShot(const World& world) const;

  // HUD 표시용
  int DashReady() const { return std::max(0, charges_); }
  float DashRefill(const World& world) const;  // 다음 칸이 차는 정도 0~1
  float ChargeRatio() const;                   // 차지샷 모으는 정도 0~1 (최대 단계까지)
  int ChargeTier() const;                      // 차지샷 지금 단계 0~4 (0 = 아직 안 모임)

 private:
  void Defeat(World& world);
  void Shoot(World& world, int chargeTier);  // chargeTier > 0 이면 차지샷
  void CastUltimate(World& world);
  void UpdateRain(World& world);
  void UpdateDashSkills(World& world);
  void DrawRushSlash(Graphics& gfx, Vec2 center) const;  // 그림자 돌진 칼날 연출
  void CheckHazards(World& world);
  bool TryStartDash(World& world);  // 대시 칸이 남아 있고 공중 대시 횟수를 넘지 않았으면 시작
  bool LastStand(const World& world) const;

  Intent intent_;
  int coyote_ = 0;
  int jumpBuffer_ = 0;
  int airJumpsLeft_ = 0;  // 이번 체공 중 남은 공중 점프 (착지하면 다시 채워진다)
  int invuln_ = 0;
  int cooldown_ = 0;
  int animFrame_ = 0;
  bool defeated_ = false;
  int defeatedTimer_ = 0;
  bool wearingHat_ = false;
  bool ultQueued_ = false;
  bool shield_ = false;     // 그림자 보호막: 피해 1번을 막는다 (스테이지마다)
  bool shootHeld_ = false;  // 차지샷: 직전 프레임에 공격 키를 누르고 있었나
  int charge_ = 0;          // 차지샷: 누르고 있던 프레임
  int rainTimer_ = 0;       // 섀도우 레인 필살기 남은 프레임
  Vec2 squash_{1.f, 1.f};  // 스쿼시/스트레치 배율 (가로, 세로). 매 프레임 1로 돌아간다.

  // ---- 대시: 짧게 수평으로 돌진. 대시 칸을 하나 쓰고, 칸은 시간이 지나면 다시 찬다. ----
  // 기본 대시는 무적이 아니다 (무적 대시 카드). 공중에서는 착지 전까지 정해진 횟수만.
  int dashTimer_ = 0;     // 남은 대시 프레임
  int dashGap_ = 0;       // 연속 대시 사이 최소 간격
  int dashBuffer_ = 0;    // 간격이 끝나기 직전에 누른 입력도 받아 준다
  int dashDir_ = 1;
  int charges_ = -1;      // 남은 대시 칸 (-1 = 첫 프레임에 가득 채운다)
  int rechargeTimer_ = 0;
  int airDashesUsed_ = 0;
  std::vector<const Actor*> rushHits_;  // 그림자 돌진: 이번 대시에 이미 벤 적
  struct Ghost { Vec2 center; int facing; float time; int life; bool phase; };  // phase = 무적 대시 잔상 (초록빛)
  std::vector<Ghost> ghosts_;  // 대시 잔상
};

}  // namespace gengar

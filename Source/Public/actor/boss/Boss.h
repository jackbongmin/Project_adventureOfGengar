#pragma once
// 최종 보스 "VOIDMAW". pos = 몸통 중심 좌표.
// 행동은 상태 머신(StateMachine<Boss>)으로 구성된다: Enter -> Float -> (공격 패턴 상태) -> Float ...
// 공격 패턴 상태는 state/boss/BossStates.h, 패턴 순서는 data/BossPatterns.h 참고.
#include <memory>

#include "actor/Actor.h"
#include "actor/Damage.h"
#include "data/BossPatterns.h"
#include "state/StateMachine.h"

namespace gengar {

struct BeamInfo {
  bool active = false;
  bool firing = false;  // false = 조준선 예고, true = 발사 중
  float angle = 0.f;
  int sweepDir = 1;
};

class Boss : public Actor, public IDamageable {
 public:
  Boss(Vec2 center, int maxHp);

  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;
  Rect Bounds() const override { return {pos.x - 52.f, pos.y - 52.f, 104.f, 104.f}; }
  void TakeDamage(World& world, const DamageInfo& info) override;
  bool IsAlive() const override { return hp_ > 0; }

  // ---- 상태(State)들이 사용하는 API ----
  World& GetWorld() { return *world_; }
  StateMachine<Boss>& Machine() { return machine_; }
  int Phase() const { return phase_; }          // 0~2 (체력 66%/33%에서 상승)
  float PhaseSpeed() const;                      // 단계별 이동 속도
  void StartAttack(BossAttack attack);           // 공격 상태로 전환 + 이름 표시
  void ToFloat();                                // 떠 있는 기본 상태로 복귀 (플래그 초기화)
  void FloatMove();                              // 플레이어 주변을 떠다니는 이동
  void FireOrb(Vec2 from, float angle, float speed);
  void FireHomingOrb(Vec2 from, float angle, float speed, float turnPerFrame, int lifeFrames);
  Vec2 PlayerCenter() const;
  Vec2 HatHandPos() const;  // 모자를 쥔 오른손의 현재 위치

  int Hp() const { return static_cast<int>(hp_ + 0.999f); }
  int MaxHp() const { return static_cast<int>(maxHp_); }
  void SetHealthRatio(float ratio) { hp_ = maxHp_ * Clamp(ratio, 0.01f, 1.f); }  // 디버그/튜닝용
  const char* AttackName() const { return attackName_; }
  int AttackNameTimer() const { return attackNameTimer_; }

  // 상태가 직접 조작하는 공개 필드
  bool invulnerable = false;     // 등장/순간이동 중 무적
  bool contactEnabled = false;   // 몸통 접촉 피해 여부 (돌진/내려찍기 등 "몸으로 덮치는" 기술 중에만 true)
  bool dying = false;
  float alpha = 1.f;
  float charge = 0.f;            // 0~1 기 모으기 연출
  float damageMultiplier = 1.f;  // 내려찍기/돌진 후 빈틈에 1.5배
  int attackIndex = 0;           // 직전에 고른 패턴의 목록 위치 (연속 반복 방지용)
  bool holdingHat = true;        // 훔쳐 간 팬텀의 보물 모자를 오른손에 쥐고 있다
  float handReach = 0.f;         // 0~1: 오른손을 handTarget 쪽으로 뻗는 정도 (격파 후 모자를 돌려주는 연출)
  Vec2 handTarget;
  BeamInfo beam;

 private:
  World* world_ = nullptr;
  int sideDir_ = 1;  // 플레이어 기준 보스가 있는 쪽(+1 오른쪽, -1 왼쪽)
  StateMachine<Boss> machine_;
  float hp_;
  float maxHp_;
  int phase_ = 0;
  int tick_ = 0;
  int flash_ = 0;
  Vec2 look_;
  const char* attackName_ = "";
  int attackNameTimer_ = 0;
};

}  // namespace gengar

#pragma once
// 피해를 받을 수 있는 객체의 공통 인터페이스.

namespace gengar {

class World;

struct DamageInfo {
  float amount = 1.f;
  int dir = 1;            // 피해를 준 쪽에서 받는 쪽으로의 x 방향 (넉백/연출용)
  bool stomp = false;     // 밟기 공격
  bool ultimate = false;  // 필살기 (게이지 충전 제외)
  const char* source = "";  // 피해 출처 이름 (디버그 로그용)
  bool fromPlayer = false;  // 플레이어 공격 (처형자/저주 배율 적용)
  bool crit = false;        // 치명타 (노란 숫자)
  bool curse = false;       // 맞은 대상에 저주 표식 (3초 동안 받는 피해 +25%)
  float knockback = 0.f;    // 넉백 속도 (일반 적만)
};

class IDamageable {
 public:
  virtual ~IDamageable() = default;
  virtual void TakeDamage(World& world, const DamageInfo& info) = 0;
  virtual bool IsAlive() const = 0;
};

}  // namespace gengar

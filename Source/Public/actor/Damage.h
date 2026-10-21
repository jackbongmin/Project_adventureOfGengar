#pragma once
// 피해를 받을 수 있는 객체의 공통 인터페이스.

namespace gengar {

class World;

struct DamageInfo {
  float amount = 1.f;
  int dir = 1;            // 피해를 준 쪽에서 받는 쪽으로의 x 방향 (넉백/연출용)
  bool stomp = false;     // 밟기 공격
  bool ultimate = false;  // 필살기 (게이지 충전 제외)
};

class IDamageable {
 public:
  virtual ~IDamageable() = default;
  virtual void TakeDamage(World& world, const DamageInfo& info) = 0;
  virtual bool IsAlive() const = 0;
};

}  // namespace gengar

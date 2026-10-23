#pragma once
// 일반 적의 공통 기반. 새 적은 이 클래스를 상속하고 컨트롤러만 바꿔 끼우면 된다.
//  - 이동/행동 방식 = Controller (Patrol / Chase / Turret / Fly ...)
//  - 겉모습/특수 행동 = 파생 클래스 (Render, OnShoot, StepPhysics ...)
#include "actor/Character.h"

namespace gengar {

class Enemy : public Character {
 public:
  Enemy(Vec2 pos, Vec2 size, int hp);

  void Update(World& world) override;
  void TakeDamage(World& world, const DamageInfo& info) override;
  void Push(float vx) { knockVel_ = vx; }  // 넉백/블랙홀이 밀거나 끄는 속도 (점점 줄어든다)
  bool Cursed() const { return curse_ > 0; }
  virtual int XpValue() const { return 2; }  // 처치 경험치 (기획서 '경험치' 시트)

 protected:
  // Intent를 속도로 변환 (기본: 보행형 - 좌우 이동 + 점프)
  virtual void ApplyIntent(World& world);
  // 물리 갱신 (기본: 중력 + 타일 충돌)
  virtual void StepPhysics(World& world);
  // Intent.shoot 처리 (원거리 적이 재정의)
  virtual void OnShoot(World& world, Vec2 aim);
  virtual Color DeathColor() const = 0;

  float moveSpeed_ = 1.f;
  float jumpSpeed_ = 9.5f;
  Intent intent_;

 private:
  void ContactPlayer(World& world);

  int stompGrace_ = 0;
  int curse_ = 0;          // 저주 표식 남은 프레임
  int curseCooldown_ = 0;  // 다시 저주에 걸릴 수 있을 때까지 남은 프레임
  float knockVel_ = 0.f;  // 밟힌 직후 몇 프레임은 플레이어와 겹쳐도 몸통 피해를 주지 않는다 (튕겨 나가는 중)
};

}  // namespace gengar

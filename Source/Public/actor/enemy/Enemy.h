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
  bool Poisoned() const { return poison_ > 0; }
  bool Chilled() const { return slow_ > 0; }
  virtual int XpValue() const { return 2; }  // 처치 경험치 (기획서 '경험치' 시트)
  // 정예: 체력 3배(최소 +40), 처치하면 경험치 4배 + 코인 12 + 하트 50%. World 가 금빛 테두리와 체력 바를 그린다.
  void MakeElite();
  bool Elite() const { return elite_; }
  int Warn() const { return warn_; }  // >0: 정예 스킬 예고 중 (머리 위 "!")
  int arenaTag = 0;  // 매복 결투장에서 불려 나온 적 (결투장 번호 + 1). 0 = 평소 적

 protected:
  // Intent를 속도로 변환 (기본: 보행형 - 좌우 이동 + 점프)
  virtual void ApplyIntent(World& world);
  // 물리 갱신 (기본: 중력 + 타일 충돌)
  virtual void StepPhysics(World& world);
  // Intent.shoot 처리 (원거리 적이 재정의)
  virtual void OnShoot(World& world, Vec2 aim);
  virtual Color DeathColor() const = 0;
  // 정예만 매 프레임 부른다. 종류마다 특별 스킬 (Slime/Hound/Caster/Bat/Hopper 에서 재정의)
  virtual void UpdateEliteSkill(World& world) { (void)world; }
  // 쓰러질 때 (정예 슬라임은 작은 슬라임으로 갈라진다)
  virtual void OnDeath(World& world) { (void)world; }

  int skillTimer_ = 0;  // 정예 스킬 주기
  int warn_ = 0;        // 스킬 예고 남은 프레임

  float moveSpeed_ = 1.f;
  float jumpSpeed_ = 9.5f;
  Intent intent_;

 private:
  void ContactPlayer(World& world);

  int stompGrace_ = 0;
  bool elite_ = false;
  int curse_ = 0;          // 저주 표식 남은 프레임
  int curseCooldown_ = 0;  // 다시 저주에 걸릴 수 있을 때까지 남은 프레임
  int poison_ = 0;         // 독 남은 프레임 (30프레임마다 poisonDmg_)
  float poisonDmg_ = 0.f;
  int slow_ = 0;           // 냉기 남은 프레임 (이동 속도 절반)
  float knockVel_ = 0.f;  // 밟힌 직후 몇 프레임은 플레이어와 겹쳐도 몸통 피해를 주지 않는다 (튕겨 나가는 중)
};

}  // namespace gengar

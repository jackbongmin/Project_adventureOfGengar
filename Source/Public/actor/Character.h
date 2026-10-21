#pragma once
// 체력/충돌/중력/컨트롤러를 가진 이동형 액터(플레이어, 적)의 공통 기반 클래스.
#include <memory>

#include "actor/Actor.h"
#include "actor/Damage.h"
#include "controller/Controller.h"
#include "controller/Intent.h"

namespace gengar {

class Character : public Actor, public IDamageable {
 public:
  Character(ActorKind kind, Vec2 pos, Vec2 size, int maxHp);

  void SetController(std::unique_ptr<Controller> controller) { controller_ = std::move(controller); }

  int Hp() const { return hp_; }
  int MaxHp() const { return maxHp_; }
  int Facing() const { return facing_; }
  void SetFacing(int f) { if (f != 0) facing_ = f > 0 ? 1 : -1; }
  bool OnGround() const { return onGround_; }
  bool HitWall() const { return hitWall_; }  // 직전 이동에서 벽에 막혔는가
  bool IsAlive() const override { return hp_ > 0 && !IsDead(); }

  // 컨트롤러가 설정하고 렌더러가 읽는 연출용 상태
  void SetAlerted(bool v) { alerted_ = v; }
  bool Alerted() const { return alerted_; }
  void SetTelegraph(float v) { telegraph_ = v; }  // 0~1 공격 예고
  float Telegraph() const { return telegraph_; }

  // dir 방향 앞이 낭떠러지(바닥에 있을 때) 또는 가시이면 true
  bool AheadBlocked(const World& world, int dir) const;

 protected:
  // 중력은 호출 측에서 vel.y에 더한 뒤 사용한다. x -> y 순서로 이동하며 충돌을 해결한다.
  void MoveAndCollide(const World& world);

  int hp_;
  int maxHp_;
  int facing_ = 1;
  bool onGround_ = false;
  bool hitWall_ = false;
  bool alerted_ = false;
  float telegraph_ = 0.f;
  int flash_ = 0;  // 피격 깜빡임 프레임
  std::unique_ptr<Controller> controller_;
};

}  // namespace gengar

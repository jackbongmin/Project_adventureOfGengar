#pragma once
#include "actor/enemy/Enemy.h"

namespace gengar {

// 사인파로 날아다니는 박쥐. 중력/타일 충돌 없이 Intent.flyVel로만 움직인다.
class Bat : public Enemy {
 public:
  Bat(Vec2 pos, int stage, float range);
  void Render(Graphics& gfx, const World& world) override;
  // 순찰 대신 플레이어를 향해 천천히 날아온다 (보스가 부른 박쥐). speed = px/프레임
  void MakeChaser(float speed) { chaseSpeed_ = speed; }

 protected:
  void ApplyIntent(World& world) override;
  void StepPhysics(World& world) override;
  Color DeathColor() const override { return Hex(0xff6a8a); }
  void UpdateEliteSkill(World& world) override;  // 정예: 예고 뒤 플레이어에게 급강하

 private:
  float flapPhase_;
  float chaseSpeed_ = 0.f;
  int dive_ = 0;      // 급강하 남은 프레임
  Vec2 diveVel_{};
};

}  // namespace gengar

#pragma once
#include "actor/enemy/Enemy.h"

namespace gengar {

// 사인파로 날아다니는 박쥐. 중력/타일 충돌 없이 Intent.flyVel로만 움직인다.
class Bat : public Enemy {
 public:
  Bat(Vec2 pos, int stage, float range);
  void Render(Graphics& gfx, const World& world) override;

 protected:
  void ApplyIntent(World& world) override;
  void StepPhysics(World& world) override;
  Color DeathColor() const override { return Hex(0xff6a8a); }

 private:
  float flapPhase_;
};

}  // namespace gengar

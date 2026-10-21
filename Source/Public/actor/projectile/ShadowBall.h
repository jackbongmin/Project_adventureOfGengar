#pragma once
// 플레이어의 섀도우볼. pos = 중심 좌표. 사거리(life)가 끝나면 작아지며 사라진다.
#include "actor/Actor.h"

namespace gengar {

class ShadowBall : public Actor {
 public:
  ShadowBall(Vec2 center, Vec2 velocity, float radius, float damage, int life);
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;
  Rect Bounds() const override { return {pos.x - radius_, pos.y - radius_, radius_ * 2.f, radius_ * 2.f}; }

 private:
  float radius_;
  float damage_;
  int life_;
  int age_ = 0;
};

}  // namespace gengar

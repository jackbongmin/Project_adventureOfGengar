#pragma once
// 적/보스가 쏘는 마법구. pos = 중심 좌표. 지형에 닿으면 사라지고 플레이어에게 1 피해.
#include "actor/Actor.h"

namespace gengar {

class EnemyOrb : public Actor {
 public:
  EnemyOrb(Vec2 center, Vec2 velocity);
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;
  Rect Bounds() const override { return {pos.x - radius_, pos.y - radius_, radius_ * 2.f, radius_ * 2.f}; }

 private:
  float radius_ = 8.f;
};

}  // namespace gengar

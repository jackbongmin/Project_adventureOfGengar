#pragma once
// 적/보스가 쏘는 마법구. pos = 중심 좌표. 지형에 닿으면 사라지고 플레이어에게 1 피해.
// piercing(보스 탄)이면 발판/벽돌은 뚫고 지나가며, 바닥(kGroundRow 이하)에 닿을 때만 사라진다.
#include "actor/Actor.h"

namespace gengar {

class EnemyOrb : public Actor {
 public:
  // homingTurn > 0 이면 플레이어를 향해 프레임당 최대 homingTurn(rad)만큼 방향을 틀고, lifeFrames 뒤에 사라진다.
  EnemyOrb(Vec2 center, Vec2 velocity, float homingTurn = 0.f, int lifeFrames = 0, bool piercing = false);
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;
  Rect Bounds() const override { return {pos.x - radius_, pos.y - radius_, radius_ * 2.f, radius_ * 2.f}; }

 private:
  float radius_ = 8.f;
  float homingTurn_;
  int life_;
  bool piercing_;
};

}  // namespace gengar

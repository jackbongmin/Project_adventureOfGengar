#pragma once
// 습득 아이템. pos = 중심 좌표.
#include "actor/Actor.h"

namespace gengar {

class Pickup : public Actor {
 public:
  explicit Pickup(Vec2 center) : Actor(ActorKind::Pickup) { pos = center; size = {24.f, 24.f}; }
  Rect Bounds() const override { return {pos.x - 12.f, pos.y - 12.f, 24.f, 24.f}; }
};

class Coin : public Pickup {
 public:
  explicit Coin(Vec2 center) : Pickup(center) {}
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;
};

// 체력이 가득 차 있으면 먹어지지 않고 그 자리에 남는다.
class Heart : public Pickup {
 public:
  explicit Heart(Vec2 center) : Pickup(center) {}
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;
};

}  // namespace gengar

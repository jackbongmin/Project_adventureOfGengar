#pragma once
// 보스 내려찍기 때 바닥을 따라 퍼지는 충격파. pos = 중심 좌표. 점프로 넘을 수 있다.
#include "actor/Actor.h"

namespace gengar {

class Shockwave : public Actor {
 public:
  Shockwave(Vec2 center, float speedX);
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;
  Rect Bounds() const override { return {pos.x - 13.f, pos.y - 11.f, 26.f, 22.f}; }
};

}  // namespace gengar

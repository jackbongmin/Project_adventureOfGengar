#pragma once
#include "actor/enemy/Enemy.h"

namespace gengar {

// 좌우로 순찰하는 기본 적.
class Slime : public Enemy {
 public:
  Slime(Vec2 pos, int stage);
  void Render(Graphics& gfx, const World& world) override;

 protected:
  Color DeathColor() const override { return Hex(0x6df0a0); }
};

}  // namespace gengar

#pragma once
#include "actor/enemy/Enemy.h"

namespace gengar {

// 제자리에서 플레이어를 조준해 마법구를 쏘는 후드 마법사. (TurretController)
class Caster : public Enemy {
 public:
  Caster(Vec2 pos, int stage);
  void Render(Graphics& gfx, const World& world) override;
  int XpValue() const override { return 4; }

 protected:
  void OnShoot(World& world, Vec2 aim) override;
  Color DeathColor() const override { return Hex(0xff5ca8); }

 private:
  float orbSpeed_;
};

}  // namespace gengar

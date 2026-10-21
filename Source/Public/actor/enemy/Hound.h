#pragma once
#include "actor/enemy/Enemy.h"
#include "controller/ChaseController.h"

namespace gengar {

// 범위 안에 들어오면 플레이어를 추적하는 그림자 늑대. (ChaseController)
class Hound : public Enemy {
 public:
  Hound(Vec2 pos, int stage);
  void Render(Graphics& gfx, const World& world) override;
  void ForceAlert();  // 검증용: 즉시 추적 상태로 만든다

 private:
  ChaseController* chase_ = nullptr;  // 소유는 Character 의 controller_

 protected:
  Color DeathColor() const override { return Hex(0x8a5cd0); }
};

}  // namespace gengar

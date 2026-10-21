#pragma once
#include "controller/Controller.h"

namespace gengar {

// 제자리에서 플레이어를 바라보고, 사거리 안이면 일정 간격으로 조준 사격한다. (후드 마법사)
class TurretController : public Controller {
 public:
  TurretController(int cooldownFrames, int initialDelay) : cooldown_(cooldownFrames), timer_(initialDelay) {}
  void Think(Character& self, World& world, Intent& out) override;

 private:
  int cooldown_;
  int timer_;
};

}  // namespace gengar

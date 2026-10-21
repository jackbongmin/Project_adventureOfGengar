#pragma once
#include "controller/Controller.h"

namespace gengar {

// 좌우로 걷다가 벽/낭떠러지/가시 앞에서 방향을 바꾼다. (슬라임)
class PatrolController : public Controller {
 public:
  explicit PatrolController(int startDir = -1) : dir_(startDir) {}
  void Think(Character& self, World& world, Intent& out) override;

 private:
  int dir_;
};

}  // namespace gengar

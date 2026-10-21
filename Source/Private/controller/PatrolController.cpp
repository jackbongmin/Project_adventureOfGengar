#include "controller/PatrolController.h"

#include "actor/Character.h"
#include "world/World.h"

namespace gengar {

void PatrolController::Think(Character& self, World& world, Intent& out) {
  if (self.HitWall() || self.AheadBlocked(world, dir_)) dir_ = -dir_;
  out.moveX = static_cast<float>(dir_);
}

}  // namespace gengar

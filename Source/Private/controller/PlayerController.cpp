#include "controller/PlayerController.h"

#include "platform/Input.h"
#include "world/World.h"

namespace gengar {

void PlayerController::Think(Character&, World& world, Intent& out) {
  const Input& in = world.In();
  out.moveX = (in.Down(Action::Right) ? 1.f : 0.f) - (in.Down(Action::Left) ? 1.f : 0.f);
  out.jumpPressed = in.Pressed(Action::Jump);
  out.jumpHeld = in.Down(Action::Jump);
  out.shoot = in.Down(Action::Shoot);
  out.ultimate = in.Pressed(Action::Ultimate);
}

}  // namespace gengar

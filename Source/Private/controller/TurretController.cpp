#include "controller/TurretController.h"

#include <cmath>

#include "actor/Character.h"
#include "actor/Player.h"
#include "world/World.h"

namespace gengar {

namespace {
constexpr float kRangeX = 460.f, kRangeY = 260.f;
constexpr int kTelegraphFrames = 35;
}  // namespace

void TurretController::Think(Character& self, World& world, Intent& out) {
  const Player& player = world.GetPlayer();
  const Rect b = self.Bounds();
  const float dx = player.Bounds().Center().x - (b.x + b.w * 0.5f);
  const float dy = player.Bounds().Center().y - (b.y + 10.f);

  if (player.IsAlive()) self.SetFacing(Sign(dx));

  if (player.IsAlive() && std::fabs(dx) < kRangeX && std::fabs(dy) < kRangeY) {
    if (--timer_ <= 0) {
      const float len = std::sqrt(dx * dx + dy * dy);
      out.shoot = true;
      out.aim = len > 0.001f ? Vec2{dx / len, dy / len} : Vec2{static_cast<float>(self.Facing()), 0.f};
      timer_ = cooldown_;
    }
  } else {
    timer_ = std::max(timer_, 60);
  }
  self.SetTelegraph(timer_ < kTelegraphFrames ? 1.f - static_cast<float>(timer_) / kTelegraphFrames : 0.f);
}

}  // namespace gengar

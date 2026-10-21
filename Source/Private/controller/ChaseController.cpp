#include "controller/ChaseController.h"

#include <cmath>

#include "actor/Character.h"
#include "actor/Player.h"
#include "platform/Audio.h"
#include "world/World.h"

namespace gengar {

namespace {
constexpr float kAggroX = 280.f, kAggroY = 110.f;  // 이 범위 안이면 추적 시작
constexpr float kLoseX = 440.f, kLoseY = 220.f;    // 이 범위 밖이면 추적 포기
}  // namespace

void ChaseController::Think(Character& self, World& world, Intent& out) {
  const Player& player = world.GetPlayer();
  const Vec2 pc = player.Bounds().Center(), ec = self.Bounds().Center();
  const float dx = pc.x - ec.x, dy = pc.y - ec.y;

  if (player.IsAlive() && !aggro_ && std::fabs(dx) < kAggroX && std::fabs(dy) < kAggroY) {
    aggro_ = true;
    world.Fx().Pop({ec.x, self.pos.y - 6.f}, "!", Hex(0xff6a8a));
    world.Snd().Play(Sfx::Alert);
  } else if (aggro_ && (!player.IsAlive() || std::fabs(dx) > kLoseX || std::fabs(dy) > kLoseY)) {
    aggro_ = false;
  }
  self.SetAlerted(aggro_);

  const int dir = aggro_ ? (Sign(dx) != 0 ? Sign(dx) : self.Facing()) : patrolDir_;
  const bool blocked = self.AheadBlocked(world, dir);

  if (aggro_) {
    self.SetFacing(dir);
    out.moveX = blocked ? 0.f : static_cast<float>(dir);
    if (self.HitWall()) out.jumpPressed = true;  // 1~2칸 벽은 뛰어넘는다
  } else {
    if (blocked || self.HitWall()) patrolDir_ = -patrolDir_;
    out.moveX = static_cast<float>(patrolDir_) * patrolRatio_;
  }
}

}  // namespace gengar

#include "controller/ChaseController.h"

#include <cmath>

#include "actor/Character.h"
#include "actor/Player.h"
#include "core/DebugOptions.h"
#include "platform/Audio.h"
#include "world/World.h"

namespace gengar {

namespace {
// 추적 시작: 같은 높이에 있을 때만 (위 발판에 올라간 플레이어는 잡지 않는다)
constexpr float kAggroX = 280.f, kAggroY = 70.f;
// 즉시 포기: 아주 멀어졌을 때
constexpr float kLoseX = 440.f, kLoseY = 220.f;
// 높이 차이가 kLeaveY 를 넘는 상태가 kLeaveFrames 동안 이어지면 포기한다. (점프로 잠깐 높아지는 건 무시)
constexpr float kLeaveY = 90.f;
constexpr int kLeaveFrames = 60;       // 약 1초
constexpr int kCooldownFrames = 150;   // 포기한 뒤 약 2.5초 동안은 다시 잡지 않는다
}  // namespace

void ChaseController::GiveUp(const char* reason) {
  aggro_ = false;
  offLevelFrames_ = 0;
  cooldown_ = kCooldownFrames;
  DebugLog("[hound] gave up (%s)", reason);
}

void ChaseController::Think(Character& self, World& world, Intent& out) {
  const Player& player = world.GetPlayer();
  const Vec2 pc = player.Bounds().Center(), ec = self.Bounds().Center();
  const float dx = pc.x - ec.x, dy = pc.y - ec.y;
  if (cooldown_ > 0) --cooldown_;

  if (forceAggro_) {  // 검증용: 높이 조건과 상관없이 추적 상태로 시작
    forceAggro_ = false;
    aggro_ = true;
    offLevelFrames_ = 0;
    DebugLog("[hound] alerted (forced) dy=%d", static_cast<int>(dy));
  } else if (player.IsAlive() && !aggro_ && cooldown_ == 0 && std::fabs(dx) < kAggroX && std::fabs(dy) < kAggroY) {
    aggro_ = true;
    offLevelFrames_ = 0;
    DebugLog("[hound] alerted at x=%d", static_cast<int>(ec.x));
    world.Fx().Pop({ec.x, self.pos.y - 6.f}, "!", Hex(0xff6a8a));
    world.Snd().Play(Sfx::Alert);
  } else if (aggro_) {
    if (!player.IsAlive()) GiveUp("player down");
    else if (std::fabs(dx) > kLoseX || std::fabs(dy) > kLoseY) GiveUp("too far");
    else if (std::fabs(dy) > kLeaveY) {
      if (++offLevelFrames_ >= kLeaveFrames) GiveUp("target on a different height");
    } else {
      offLevelFrames_ = 0;
    }
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

#include "controller/BotController.h"

#include <cmath>

#include "actor/Character.h"
#include "actor/boss/Boss.h"
#include "core/Config.h"
#include "core/Session.h"
#include "world/World.h"

namespace gengar {

void BotController::Think(Character& self, World& w, Intent& out) {
  const Level& lv = w.GetLevel();
  const float T = static_cast<float>(cfg::kTile);
  const Rect b = self.Bounds();

  out.shoot = !w.Sess().Has(CardId::ChargeShot) || w.Frame() % 75 != 0;  // 차지샷: 75프레임마다 떼서 발사
  out.jumpHeld = true;
  out.ultimate = w.Sess().gauge >= 100.f;

  const Boss* boss = w.GetBoss();
  if (boss && boss->IsAlive() && !boss->dying) {
    const float dx = boss->pos.x - b.Center().x;
    if (std::fabs(dx) < 140.f) retreating_ = true;
    else if (std::fabs(dx) > 240.f) retreating_ = false;
    const float toward = dx > 0.f ? 1.f : -1.f;
    out.moveX = retreating_ ? -toward : toward;
    out.jumpPressed = self.OnGround();  // 보스는 공중에 떠 있으므로 계속 뛰면서 쏜다
    return;
  }

  out.moveX = 1.f;
  const int aheadCol = static_cast<int>(std::floor((b.Right() + 6.f) / T));
  const int feetRow = static_cast<int>(std::floor((b.Bottom() + 2.f) / T));
  bool jump = self.HitWall();
  if (self.OnGround()) {
    if (!lv.Solid(aheadCol, feetRow)) jump = true;                    // 낭떠러지
    if (lv.At(aheadCol, feetRow - 1) == Tile::Spike) jump = true;     // 가시
  }
  out.jumpPressed = jump;
}

}  // namespace gengar

#include "controller/BotController.h"

#include <algorithm>
#include <cmath>

#include "actor/Character.h"
#include "actor/boss/Boss.h"
#include "actor/enemy/Enemy.h"
#include "actor/pickup/Pickups.h"
#include "core/DebugOptions.h"
#include "core/Config.h"
#include "core/Session.h"
#include "world/World.h"

namespace gengar {

void BotController::Think(Character& self, World& w, Intent& out) {
  const Level& lv = w.GetLevel();
  const float T = static_cast<float>(cfg::kTile);
  const Rect b = self.Bounds();

  const bool charges = w.Sess().Has(CardId::ChargeShot) || w.Sess().Has(CardId::Brimstone);
  out.shoot = !charges || w.Frame() % 160 != 0;  // 차지샷: 최대 단계(150프레임)까지 모았다가 떼서 발사
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

  if (--retarget_ <= 0 || (w.ArenaActive() != (goal_ == Goal::Fight))) { ChooseGoal(self, w); retarget_ = kRetargetFrames; }
  switch (goal_) {
    case Goal::Fight: ThinkArena(self, w, out); return;
    case Goal::Chest: ThinkChest(self, w, out); return;
    case Goal::Flag: WalkTo(self, w, goalX_, out); return;
    case Goal::Advance: break;
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

// 목표 우선순위: 결투장 적 > 가까운(500px 안) 닫힌 보물상자 > 지나쳐 버린 깃발 > 전진
void BotController::ChooseGoal(const Character& self, World& w) {
  const Goal before = goal_;
  const Vec2 me = self.Bounds().Center();
  if (w.ArenaActive()) {
    goal_ = Goal::Fight;
  } else {
    goal_ = Goal::Advance;
    const Level& lv = w.GetLevel();
    const float T = static_cast<float>(cfg::kTile);
    const float prevChest = goalX_;
    float best = 500.f;
    w.ForEach(ActorKind::Pickup, [&](Actor& a) {
      const Chest* chest = dynamic_cast<const Chest*>(&a);
      if (!chest || chest->Opened()) return;
      if (std::any_of(givenUp_.begin(), givenUp_.end(), [&](float x) { return std::fabs(x - a.pos.x) < 4.f; })) return;
      const float d = std::fabs(a.pos.x - me.x);
      if (d >= best) return;
      // 점프(약 3칸)로 닿지 않는 높이면 상자 양옆 12칸 안의 점프 버섯을 찾는다. 없으면 갈 수 없는 상자
      float spring = -1.f;
      if (chest->Landed() && a.pos.y < me.y - 100.f) {  // 떨어지는 중인 상자(결투장 보상)는 곧 바닥에 앉으니 그냥 간다
        const int cc = static_cast<int>(a.pos.x / T);
        for (int k = 0; k <= 12 && spring < 0.f; ++k) {
          for (int c : {cc - k, cc + k}) {
            if (lv.At(c, cfg::kGroundRow) == Tile::Spring) { spring = (static_cast<float>(c) + 0.5f) * T; break; }
          }
        }
        if (spring < 0.f) return;
      }
      best = d;
      goal_ = Goal::Chest;
      goalX_ = a.pos.x;
      chestY_ = a.pos.y;
      springX_ = spring;
    });
    if (goal_ == Goal::Chest && std::fabs(goalX_ - prevChest) > 4.f) chaseFrames_ = 0;
    if (goal_ == Goal::Advance && lv.HasFlag() && me.x > lv.Flag().Right() + 8.f) { goal_ = Goal::Flag; goalX_ = lv.Flag().x + lv.Flag().w * 0.5f; }
  }
  if (goal_ != before) DebugLog("[bot] goal %d -> %d (x=%d)", static_cast<int>(before), static_cast<int>(goal_), static_cast<int>(me.x));
}

// 지면을 따라 x 까지 걸어간다. 벽이나 낭떠러지·가시 앞에서는 점프.
void BotController::WalkTo(Character& self, World& w, float x, Intent& out) {
  const Level& lv = w.GetLevel();
  const float T = static_cast<float>(cfg::kTile);
  const Rect b = self.Bounds();
  const float dx = x - b.Center().x;
  const int dir = dx >= 0.f ? 1 : -1;
  out.moveX = std::fabs(dx) > 6.f ? static_cast<float>(dir) : 0.f;
  const int aheadCol = static_cast<int>(std::floor((dir > 0 ? b.Right() + 6.f : b.x - 6.f) / T));
  const int feetRow = static_cast<int>(std::floor((b.Bottom() + 2.f) / T));
  bool jump = self.HitWall();
  if (self.OnGround()) {
    if (!lv.Solid(aheadCol, feetRow)) jump = true;
    if (lv.At(aheadCol, feetRow - 1) == Tile::Spike) jump = true;
  }
  out.jumpPressed = jump && self.OnGround();
}

void BotController::ThinkChest(Character& self, World& w, Intent& out) {
  if (++chaseFrames_ > 900) {  // 15초 안에 못 열면 포기
    DebugLog("[bot] give up chest at x=%d", static_cast<int>(goalX_));
    givenUp_.push_back(goalX_);
    goal_ = Goal::Advance;
    retarget_ = 0;
    return;
  }
  const Rect b = self.Bounds();
  const float dx = goalX_ - b.Center().x;
  const bool high = chestY_ < b.Center().y - 100.f;  // 지금 서 있는 곳에서 점프로 닿지 않는다
  if (!self.OnGround()) {  // 공중(버섯으로 튀어 오르는 중 포함): 상자 쪽으로 몸을 튼다
    out.moveX = std::fabs(dx) > 6.f ? (dx > 0.f ? 1.f : -1.f) : 0.f;
    return;
  }
  if (high && springX_ >= 0.f) { WalkTo(self, w, springX_, out); return; }  // 버섯을 밟으면 저절로 튀어 오른다
  WalkTo(self, w, goalX_, out);
  // 상자가 살짝 위(발판 위)에 있으면 바로 아래에서 한 번 뛴다
  if (std::fabs(dx) < 40.f && chestY_ < b.Center().y - 20.f) out.jumpPressed = true;
}

// ① 목표: 결투장에서 불려 나온 적 가운데 가장 가까운 것. 벽 너머의 평소 적은 맞힐 수 없으니 고르지 않는다.
//    같은 높이(위아래 60px 안)의 적은 거리를 반으로 쳐서 먼저 고른다 (옮겨 다니지 않고 바로 쏠 수 있다).
const Actor* BotController::PickArenaTarget(const Character& self, World& w) {
  const Vec2 me = self.Bounds().Center();
  const Actor* best = nullptr;
  float bestScore = 1e9f;
  w.ForEach(ActorKind::Enemy, [&](Actor& a) {
    if (static_cast<Enemy&>(a).arenaTag == 0) return;
    const Vec2 c = a.Bounds().Center();
    float score = std::hypot(c.x - me.x, c.y - me.y);
    if (std::fabs(c.y - me.y) < 60.f) score *= 0.5f;
    if (score < bestScore) { bestScore = score; best = &a; }
  });
  return best;
}

void BotController::ThinkArena(Character& self, World& w, Intent& out) {
  const Actor* target = PickArenaTarget(self, w);
  if (!target) return;  // 다음 웨이브를 기다린다
  const Vec2 me = self.Bounds().Center(), c = target->Bounds().Center();
  const float dx = c.x - me.x, dy = c.y - me.y;
  const int toward = dx >= 0.f ? 1 : -1;

  // ② 높이 맞추기
  if (dy < -50.f) {  // 위: 목표 아래로 다가가며 뛰어오른다 (뜬 발판은 아래에서 통과해 올라선다)
    out.moveX = std::fabs(dx) > 40.f ? static_cast<float>(toward) : 0.f;
    out.jumpPressed = self.OnGround();
    return;
  }
  if (dy > 60.f && self.OnGround()) {  // 아래: 발판 위라면 목표 쪽 끝으로 걸어가 떨어진다
    out.moveX = static_cast<float>(toward);
    return;
  }

  // ③ 거리 맞추기: 너무 멀면 다가가고, 너무 가까우면 물러서되 마지막에는 목표 쪽을 보게 한다
  const float dist = std::fabs(dx);
  if (dist > 260.f) out.moveX = static_cast<float>(toward);
  else if (dist < 120.f && !self.HitWall()) out.moveX = -static_cast<float>(toward);
  else out.moveX = self.Facing() == toward ? 0.f : static_cast<float>(toward) * 0.2f;
  // 물러서는 중에는 등을 보이므로, 3프레임마다 한 번은 목표 쪽으로 돌아서서 쏜다
  if (out.moveX == -static_cast<float>(toward) && w.Frame() % 3 == 0) out.moveX = static_cast<float>(toward) * 0.2f;
  out.jumpPressed = self.OnGround() && self.HitWall();
  // 그림자 돌진이 있으면 가까운 적에게 대시로 벤다 (결투장 안에는 구덩이가 없다)
  out.dash = w.Sess().Has(CardId::ShadowRush) && dist < 160.f && std::fabs(dy) < 40.f && w.Frame() % 45 == 0;
}

}  // namespace gengar

#pragma once
// 습득 아이템. pos = 중심 좌표.
#include "actor/Actor.h"

namespace gengar {

class Pickup : public Actor {
 public:
  explicit Pickup(Vec2 center) : Actor(ActorKind::Pickup) { pos = center; size = {24.f, 24.f}; }
  Rect Bounds() const override { return {pos.x - 12.f, pos.y - 12.f, 24.f, 24.f}; }
};

class Coin : public Pickup {
 public:
  explicit Coin(Vec2 center) : Pickup(center) {}
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;
};

// 체력이 가득 차 있으면 먹어지지 않고 그 자리에 남는다.
class Heart : public Pickup {
 public:
  explicit Heart(Vec2 center) : Pickup(center) {}
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;

 private:
  bool skipLogged_ = false;  // 디버그 로그 중복 방지
};

// 보스가 격파되며 돌려준 팬텀의 보물 모자. 천천히 내려앉아 빛나고,
// 보스가 완전히 사라진 뒤 플레이어가 닿으면 World::NotifyHatCollected() -> 엔딩.
// 오래 줍지 않으면(봇 포함) 플레이어에게 끌려온다.
class TreasureHat : public Pickup {
 public:
  explicit TreasureHat(Vec2 center) : Pickup(center) {}
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;

 private:
  bool settled_ = false;
  int settledFrames_ = 0;
};

}  // namespace gengar

#pragma once
#include "actor/enemy/Enemy.h"

namespace gengar {

// 좌우로 순찰하는 기본 적.
// 정예: 3초마다 "!" 예고 뒤 플레이어 쪽으로 크게 뛰어들고, 쓰러지면 체력 1짜리 작은 슬라임 셋으로 갈라진다.
class Slime : public Enemy {
 public:
  Slime(Vec2 pos, int stage);
  void Render(Graphics& gfx, const World& world) override;

 protected:
  Color DeathColor() const override { return Hex(0x6df0a0); }
  void ApplyIntent(World& world) override;
  void UpdateEliteSkill(World& world) override;
  void OnDeath(World& world) override;

 private:
  int stage_;
  bool hop_ = false;   // 정예 뛰어들기 중
  float hopVx_ = 0.f;
};

}  // namespace gengar

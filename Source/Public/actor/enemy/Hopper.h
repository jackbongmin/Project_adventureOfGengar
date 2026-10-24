#pragma once
#include "actor/enemy/Enemy.h"

namespace gengar {

// 그림자 두꺼비. 웅크렸다가 플레이어 쪽으로 크게 뛰어든다 (가까이 없으면 제자리에서 작게 뛴다).
// 뛰기 전에 떨어질 자리를 보고, 그 아래가 구덩이면 덜 뛰거나 제자리에서 뛴다 (절벽으로 떨어지지 않는다).
// 정예는 뛰기 전에 "!" 로 예고하고, 착지할 때 좌우로 짧은 충격파를 낸다.
class Hopper : public Enemy {
 public:
  Hopper(Vec2 pos, int stage);
  void Render(Graphics& gfx, const World& world) override;
  int XpValue() const override { return 3; }

 protected:
  void ApplyIntent(World& world) override;
  void UpdateEliteSkill(World& world) override;
  Color DeathColor() const override { return Hex(0x5ce0c0); }

 private:
  int wait_;              // 다음 점프까지 남은 프레임 (땅에 있을 때)
  bool air_ = false;      // 점프 중
  bool landed_ = false;   // 이번 프레임에 막 착지 (정예 충격파)
  float jumpVx_ = 0.f;

  // 가로 속도 vx 로 뛰었을 때 떨어질 자리 아래에 땅이 있는가 (구덩이 = 화면 아래까지 빈칸)
  bool SafeJump(const World& world, float vx, float vy) const;
};

}  // namespace gengar

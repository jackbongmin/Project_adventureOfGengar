#pragma once
// 플레이어의 섀도우볼. pos = 중심 좌표. 사거리(life)가 끝나면 작아지며 사라진다.
// 날아가는 동안 지나온 자리에 잔상을 남기고, 발사 직후 출발점에 작은 도트 고리가 퍼진다.
#include "actor/Actor.h"

namespace gengar {

class ShadowBall : public Actor {
 public:
  ShadowBall(Vec2 center, Vec2 velocity, float radius, float damage, int life);
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;
  Rect Bounds() const override { return {pos.x - radius_, pos.y - radius_, radius_ * 2.f, radius_ * 2.f}; }

 private:
  static constexpr int kGhosts = 3;       // 잔상 개수
  static constexpr int kGhostStride = 2;  // 잔상 하나당 몇 프레임 전 위치인가
  static constexpr int kHistory = kGhosts * kGhostStride;

  float radius_;
  float damage_;
  int life_;
  int age_ = 0;
  Vec2 origin_;
  Vec2 history_[kHistory] = {};  // 최근 위치 (원형 버퍼, age_ 로 인덱스)
};

}  // namespace gengar

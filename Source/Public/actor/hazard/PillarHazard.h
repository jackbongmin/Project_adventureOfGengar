#pragma once
// 보스의 "그림자 기둥": 바닥에 마법진 예고(warn 프레임) 후 기둥이 솟는다. pos.x = 중심 x.
#include "actor/Actor.h"

namespace gengar {

class PillarHazard : public Actor {
 public:
  explicit PillarHazard(float centerX, int warnFrames = 50);
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;
  Rect Bounds() const override;

 private:
  static constexpr int kActiveFrames = 22;
  int warn_;
  int t_ = 0;
};

}  // namespace gengar

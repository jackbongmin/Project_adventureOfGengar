#include "controller/FlyController.h"

#include <cmath>

#include "actor/Character.h"

namespace gengar {

void FlyController::Think(Character& self, World&, Intent& out) {
  ++t_;
  const float a = static_cast<float>(t_) * 0.025f + phase_;
  const float b = static_cast<float>(t_) * 0.07f + phase_;
  // 위치 = 앵커 + range*sin(a) 의 시간 미분(속도)을 전달한다.
  out.flyVel = {range_ * 0.025f * std::cos(a), 30.f * 0.07f * std::cos(b)};
  self.SetFacing(std::cos(a) >= 0.f ? 1 : -1);
}

}  // namespace gengar

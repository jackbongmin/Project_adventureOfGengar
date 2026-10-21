#pragma once
#include "core/Config.h"
#include "core/Types.h"

namespace gengar {

struct Camera {
  float x = 0.f;

  void Update(const Rect& target, int facing, float levelPixelWidth) {
    const float maxX = std::max(0.f, levelPixelWidth - static_cast<float>(cfg::kScreenW));
    const float want = Clamp(target.Center().x - cfg::kScreenW * 0.5f + static_cast<float>(facing) * 40.f, 0.f, maxX);
    x += (want - x) * 0.12f;
  }
};

}  // namespace gengar

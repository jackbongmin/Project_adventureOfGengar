#pragma once
#include "core/Config.h"
#include "core/Types.h"

namespace gengar {

struct Camera {
  float x = 0.f;

  // lockMin/lockMax >= 0 이면 카메라 왼쪽 끝을 그 사이로 묶는다 (매복 결투장)
  void Update(const Rect& target, int facing, float levelPixelWidth, float lockMin = -1.f, float lockMax = -1.f) {
    const float maxX = std::max(0.f, levelPixelWidth - static_cast<float>(cfg::kScreenW));
    float want = Clamp(target.Center().x - cfg::kScreenW * 0.5f + static_cast<float>(facing) * 40.f, 0.f, maxX);
    if (lockMin >= 0.f) want = Clamp(want, lockMin, std::max(lockMin, lockMax));
    x += (want - x) * 0.12f;
  }
};

}  // namespace gengar

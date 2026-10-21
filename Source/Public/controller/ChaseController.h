#pragma once
#include "controller/Controller.h"

namespace gengar {

// 평소에는 순찰, 범위 안에 플레이어가 들어오면 추적, 멀어지면 포기. (그림자 늑대)
class ChaseController : public Controller {
 public:
  // patrolRatio: 순찰 속도 / 추적 속도 (moveX 배율)
  explicit ChaseController(float patrolRatio) : patrolRatio_(patrolRatio) {}
  void Think(Character& self, World& world, Intent& out) override;

 private:
  float patrolRatio_;
  int patrolDir_ = -1;
  bool aggro_ = false;
};

}  // namespace gengar

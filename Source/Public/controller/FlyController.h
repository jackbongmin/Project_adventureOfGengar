#pragma once
#include "controller/Controller.h"

namespace gengar {

// 사인파로 좌우/상하를 오가며 비행한다. (박쥐)
class FlyController : public Controller {
 public:
  FlyController(float range, float phase) : range_(range), phase_(phase) {}
  void Think(Character& self, World& world, Intent& out) override;

 private:
  float range_;
  float phase_;
  int t_ = 0;
};

}  // namespace gengar

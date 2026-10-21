#pragma once
#include "controller/Controller.h"

namespace gengar {

// 키보드 입력(Input)을 Intent로 변환한다.
class PlayerController : public Controller {
 public:
  void Think(Character& self, World& world, Intent& out) override;
};

}  // namespace gengar

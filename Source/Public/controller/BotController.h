#pragma once
#include "controller/Controller.h"

namespace gengar {

// 자동 플레이 봇 (검증용, 디버그 옵션 bot=1). 사람이 키보드로 하는 것과 같은 Intent를 만든다.
//  - 플랫폼 구간: 오른쪽으로 달리며 벽/낭떠러지/가시 앞에서 점프, 계속 공격, 게이지가 차면 필살기
//  - 보스전: 보스와 140~240px 거리를 오가며 계속 점프하면서 공격 (몸 안으로 들어가지 않는다)
class BotController : public Controller {
 public:
  void Think(Character& self, World& world, Intent& out) override;

 private:
  bool retreating_ = false;  // 보스전: 접근 <-> 후퇴를 오가며 거리를 유지한다
};

}  // namespace gengar

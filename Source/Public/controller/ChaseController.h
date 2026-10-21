#pragma once
#include "controller/Controller.h"

namespace gengar {

// 평소에는 순찰, 같은 높이의 플레이어가 범위 안에 들어오면 추적한다. (그림자 늑대)
// 추적을 포기하는 경우: 너무 멀어짐 / 플레이어 사망 / 높이 차이가 일정 시간 이상 이어짐(위 발판 등).
// 포기한 직후에는 잠시 다시 잡지 않아서 위아래를 오가며 유인하기 쉽다.
class ChaseController : public Controller {
 public:
  // patrolRatio: 순찰 속도 / 추적 속도 (moveX 배율)
  explicit ChaseController(float patrolRatio) : patrolRatio_(patrolRatio) {}
  void Think(Character& self, World& world, Intent& out) override;

  void ForceAggro() { forceAggro_ = true; }  // 검증용: 다음 프레임에 무조건 추적 상태로 시작

 private:
  void GiveUp(const char* reason);

  float patrolRatio_;
  int patrolDir_ = -1;
  bool aggro_ = false;
  bool forceAggro_ = false;
  int offLevelFrames_ = 0;  // 높이 차이가 큰 상태로 이어진 프레임 수
  int cooldown_ = 0;        // 포기한 뒤 재추적 금지 시간
};

}  // namespace gengar

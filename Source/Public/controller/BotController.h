#pragma once
#include <vector>

#include "controller/Controller.h"

namespace gengar {

// 자동 플레이 봇 (검증용, 디버그 옵션 bot=1). 사람이 키보드로 하는 것과 같은 Intent를 만든다.
//  - 플랫폼 구간: 오른쪽으로 달리며 벽/낭떠러지/가시 앞에서 점프, 계속 공격, 게이지가 차면 필살기
//  - 보스전: 보스와 140~240px 거리를 오가며 계속 점프하면서 공격 (몸 안으로 들어가지 않는다)
//  - 매복 결투장: ① 목표 고르기(결투장 적만) ② 높이 맞추기(위면 뛰어올라 발판, 아래면 발판 끝으로 내려감)
//                 ③ 거리 맞추기(120~260px, 목표 쪽을 본다) ④ 계속 공격
//  - 목표는 kRetargetFrames 마다 우선순위대로 다시 고른다:
//      1 결투장 적 > 2 가까운 보물상자 > 3 지나쳐 버린 깃발(되돌아간다) > 4 오른쪽으로 전진
//  - 보물상자: 점프로 닿는 높이면 걸어가서 열고, 높으면 근처 점프 버섯을 밟아 튀어 올라 공중에서 상자 쪽으로 간다.
//              버섯이 없거나 15초 안에 못 열면 그 상자는 포기한다.
class Actor;

class BotController : public Controller {
 public:
  void Think(Character& self, World& world, Intent& out) override;

 private:
  enum class Goal { Advance, Fight, Chest, Flag };
  static constexpr int kRetargetFrames = 30;

  void ChooseGoal(const Character& self, World& world);
  void ThinkArena(Character& self, World& world, Intent& out);
  void ThinkChest(Character& self, World& world, Intent& out);
  void WalkTo(Character& self, World& world, float x, Intent& out);  // 지면을 따라 x 로 걸어간다 (벽/낭떠러지 앞에서 점프)
  static const Actor* PickArenaTarget(const Character& self, World& world);

  Goal goal_ = Goal::Advance;
  float goalX_ = 0.f;      // 상자/깃발로 갈 때의 목표 x
  int retarget_ = 0;       // 다음 재탐색까지 남은 프레임
  float chestY_ = 0.f;     // 목표 상자의 y (goalX_ 와 함께)
  float springX_ = -1.f;   // 높은 상자로 가려고 밟을 점프 버섯의 x (-1 = 필요 없음)
  int chaseFrames_ = 0;    // 지금 상자를 쫓은 시간
  std::vector<float> givenUp_;  // 포기한 상자의 x

  bool retreating_ = false;  // 보스전: 접근 <-> 후퇴를 오가며 거리를 유지한다
};

}  // namespace gengar

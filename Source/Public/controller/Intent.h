#pragma once
// 컨트롤러(플레이어 입력 / AI)가 매 프레임 액터에게 전달하는 "의도".
// 액터는 입력 장치나 AI를 몰라도 Intent만 보고 움직인다.
#include "core/Types.h"

namespace gengar {

struct Intent {
  float moveX = 0.f;         // -1 ~ 1 (보행 액터)
  bool jumpPressed = false;  // 점프 시작
  bool jumpHeld = false;     // 점프 키 유지 (가변 점프)
  bool shoot = false;        // 공격
  bool ultimate = false;     // 필살기
  Vec2 aim;                  // 조준 방향 (단위 벡터, 원거리 공격용)
  Vec2 flyVel;               // 비행 액터의 목표 속도
};

}  // namespace gengar

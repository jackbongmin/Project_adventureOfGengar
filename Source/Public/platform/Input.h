#pragma once
// 키보드 입력을 "액션" 단위로 추상화한다. 키 배치를 바꾸려면 Input.cpp의 바인딩 표만 수정하면 된다.
#include <array>

#include "core/Types.h"

union SDL_Event;

namespace gengar {

enum class Action { Left, Right, Up, Down, Jump, Shoot, Ultimate, Confirm, Pause, Gallery, Skip, Dash, Reroll, Status, Count };

class Input {
 public:
  void OnEvent(const SDL_Event& e);
  void ClearPressed();  // 고정 스텝 업데이트 1회가 끝날 때마다 호출

  bool Down(Action a) const;     // 누르고 있는 동안
  bool Pressed(Action a) const;  // 이번 스텝에 막 눌림
  // 마우스 위치 (게임 화면 좌표 960x544, 창 크기와 상관없다). 한 번도 움직이지 않았으면 MouseSeen() = false
  Vec2 Mouse() const { return mouse_; }
  bool MouseSeen() const { return mouseSeen_; }

 private:
  static constexpr int kCount = static_cast<int>(Action::Count);
  std::array<bool, kCount> pressed_{};
  Vec2 mouse_{};
  bool mouseSeen_ = false;
};

}  // namespace gengar

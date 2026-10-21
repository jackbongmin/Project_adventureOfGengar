#pragma once
// 키보드 입력을 "액션" 단위로 추상화한다. 키 배치를 바꾸려면 Input.cpp의 바인딩 표만 수정하면 된다.
#include <array>

union SDL_Event;

namespace gengar {

enum class Action { Left, Right, Up, Down, Jump, Shoot, Ultimate, Confirm, Pause, Gallery, Skip, Dash, Count };

class Input {
 public:
  void OnEvent(const SDL_Event& e);
  void ClearPressed();  // 고정 스텝 업데이트 1회가 끝날 때마다 호출

  bool Down(Action a) const;     // 누르고 있는 동안
  bool Pressed(Action a) const;  // 이번 스텝에 막 눌림

 private:
  static constexpr int kCount = static_cast<int>(Action::Count);
  std::array<bool, kCount> pressed_{};
};

}  // namespace gengar

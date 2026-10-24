#include "platform/Input.h"

#include <SDL.h>

#include <vector>

namespace gengar {
namespace {

// 액션별 키 바인딩. 왼손은 Z(공격, 약지) X(대시, 중지) C(점프, 검지)에 얹고, 오른손은 방향키.
// 가장 자주 함께 누르는 점프+대시를 이웃한 검지/중지에 두고, 가끔 쓰는 필살기는 손을 조금 올려야 하는 A에 둔다.
const std::array<std::vector<SDL_Scancode>, static_cast<size_t>(Action::Count)>& Bindings() {
  static const std::array<std::vector<SDL_Scancode>, static_cast<size_t>(Action::Count)> table = {{
      {SDL_SCANCODE_LEFT},                                           // Left
      {SDL_SCANCODE_RIGHT},                                          // Right
      {SDL_SCANCODE_UP},                                             // Up
      {SDL_SCANCODE_DOWN},                                           // Down
      {SDL_SCANCODE_C, SDL_SCANCODE_SPACE, SDL_SCANCODE_UP},         // Jump
      {SDL_SCANCODE_Z},                                              // Shoot
      {SDL_SCANCODE_A},                                              // Ultimate
      {SDL_SCANCODE_RETURN, SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_Z},  // Confirm
      {SDL_SCANCODE_ESCAPE, SDL_SCANCODE_P},                         // Pause
      {SDL_SCANCODE_G},                                              // Gallery (개발용 스프라이트 갤러리)
      {SDL_SCANCODE_C},                                              // Skip (시네마틱에서 꾹 누르면 건너뛰기)
      {SDL_SCANCODE_X, SDL_SCANCODE_LSHIFT},                         // Dash (왼쪽 Shift = 새끼손가락 대안)
      {SDL_SCANCODE_R},                                              // Reroll (카드 다시 뽑기)
  }};
  return table;
}

}  // namespace

void Input::OnEvent(const SDL_Event& e) {
  if (e.type != SDL_KEYDOWN || e.key.repeat != 0) return;
  const auto& table = Bindings();
  for (int a = 0; a < kCount; ++a) {
    for (SDL_Scancode sc : table[static_cast<size_t>(a)]) {
      if (e.key.keysym.scancode == sc) pressed_[static_cast<size_t>(a)] = true;
    }
  }
}

void Input::ClearPressed() { pressed_.fill(false); }

bool Input::Down(Action a) const {
  const Uint8* keys = SDL_GetKeyboardState(nullptr);
  for (SDL_Scancode sc : Bindings()[static_cast<size_t>(a)]) {
    if (keys[sc]) return true;
  }
  return false;
}

bool Input::Pressed(Action a) const { return pressed_[static_cast<size_t>(a)]; }

}  // namespace gengar

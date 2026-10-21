#include "mode/GameOverMode.h"

#include <string>

#include "core/Config.h"
#include "core/Game.h"
#include "core/Session.h"
#include "platform/Graphics.h"
#include "platform/Input.h"

namespace gengar {

void GameOverMode::Update(Game& game) {
  ++frame_;
  score_ = game.Sess().score;
  if (game.In().Pressed(Action::Confirm)) game.GoTitle();
}

void GameOverMode::Render(Graphics& g) {
  constexpr float W = static_cast<float>(cfg::kScreenW);
  g.FillRect(0, 0, W, static_cast<float>(cfg::kScreenH), Hex(0x0a0214));
  g.Text("GAME OVER", W * 0.5f, 190, 9, Hex(0xff6a8a), Align::Center);
  g.Text("SCORE " + std::to_string(score_), W * 0.5f, 290, 4, Hex(0xffffff), Align::Center);
  if ((frame_ / 30) % 2 == 0) g.Text("PRESS ENTER - TITLE", W * 0.5f, 380, 3, Hex(0xffe27a), Align::Center);
}

}  // namespace gengar

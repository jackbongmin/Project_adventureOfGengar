#include "mode/VictoryMode.h"

#include <string>

#include "actor/PhantomArt.h"
#include "core/Config.h"
#include "core/Game.h"
#include "core/Session.h"
#include "platform/Graphics.h"
#include "platform/Input.h"

namespace gengar {

void VictoryMode::Update(Game& game) {
  ++frame_;
  score_ = game.Sess().score;
  if (frame_ > 30 && game.In().Pressed(Action::Confirm)) game.GoTitle();
}

void VictoryMode::Render(Graphics& g) {
  constexpr float W = static_cast<float>(cfg::kScreenW), H = static_cast<float>(cfg::kScreenH);
  for (float y = 0; y < H; y += 8.f) g.FillRect(0, y, W, 8.f, Lerp(Hex(0x120428), Hex(0x4a1a78), y / H));
  DrawPhantom(g, {W * 0.5f, 140.f}, 1, static_cast<float>(frame_), 2.6f);
  g.Text("VICTORY!", W * 0.5f, 230, 9, Hex(0xffe27a), Align::Center);
  g.Text("THE LORD OF THE VOID IS DEFEATED. PEACE RETURNS TO THE DARK.", W * 0.5f, 310, 2, Hex(0xffffff), Align::Center);
  g.Text("FINAL SCORE  " + std::to_string(score_), W * 0.5f, 360, 4, Hex(0xd9b8ff), Align::Center);
  if ((frame_ / 30) % 2 == 0) g.Text("PRESS ENTER - TITLE", W * 0.5f, 450, 3, Hex(0xffe27a), Align::Center);
}

}  // namespace gengar

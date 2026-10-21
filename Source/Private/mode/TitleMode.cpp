#include "mode/TitleMode.h"

#include <cmath>

#include "actor/PhantomArt.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "core/Game.h"
#include "platform/Graphics.h"
#include "platform/Input.h"

namespace gengar {

void TitleMode::Update(Game& game) {
  ++frame_;
  if (Debug().bot && frame_ > 30) game.StartNewRun();  // bot=1 이면 시네마틱 없이 자동 시작
  else if (game.In().Pressed(Action::Confirm)) game.GoIntro();
  if (game.In().Pressed(Action::Gallery)) game.GoGallery();
}

void TitleMode::Render(Graphics& g) {
  constexpr float W = static_cast<float>(cfg::kScreenW), H = static_cast<float>(cfg::kScreenH);
  for (float y = 0; y < H; y += 8.f) g.FillRect(0, y, W, 8.f, Lerp(Hex(0x0e0420), Hex(0x3a1260), y / H));
  for (int i = 0; i < 80; ++i) {
    const float a = 0.3f + 0.5f * std::sin(static_cast<float>(frame_) * 0.04f + static_cast<float>(i));
    g.FillRect(static_cast<float>((i * 151) % cfg::kScreenW), static_cast<float>((i * 67) % 400), 2, 2, Rgb(255, 255, 255, static_cast<int>(std::max(0.f, a) * 255)));
  }
  DrawPhantom(g, {W * 0.5f, 190.f}, 1, static_cast<float>(frame_), 3.2f);
  g.Text("ADVENTURE OF GENGAR", W * 0.5f, 292, 6, Hex(0xd9b8ff), Align::Center);
  g.Text("THE LORD OF THE VOID AWAITS", W * 0.5f, 350, 2, Hex(0xffffff), Align::Center);
  g.Text("< > MOVE   C JUMP   Z SHOT   X DASH   A ULTIMATE   ESC PAUSE", W * 0.5f, 418, 2, Hex(0xc9b0ee), Align::Center);
  g.Text("(SPACE / UP ALSO JUMP, SHIFT ALSO DASH. DASH DODGES ATTACKS!)", W * 0.5f, 442, 2, Hex(0x9a82c0), Align::Center);
  if ((frame_ / 30) % 2 == 0) g.Text("PRESS ENTER TO START", W * 0.5f, 490, 3, Hex(0xffe27a), Align::Center);
}

}  // namespace gengar

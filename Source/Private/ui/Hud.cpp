#include "ui/Hud.h"

#include <cmath>
#include <string>

#include "actor/Player.h"
#include "actor/boss/Boss.h"
#include "art/Art.h"
#include "core/Config.h"
#include "core/Session.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

void DrawHud(Graphics& g, const World& w, const Session& s) {
  constexpr float kW = static_cast<float>(cfg::kScreenW);
  const Player& p = w.GetPlayer();

  g.FillRect(0, 0, kW, 44, Rgb(10, 0, 25, 140));
  for (int i = 0; i < p.MaxHp(); ++i) g.DrawSprite(i < p.Hp() ? art::A().heartFull : art::A().heartEmpty, 12.f + static_cast<float>(i) * 28.f, 10.f, 3);
  g.Text("LIFE x" + std::to_string(s.lives), 26.f + static_cast<float>(p.MaxHp()) * 28.f + 10.f, 15, 2, Hex(0xd9c2ff));

  g.Text("$ " + std::to_string(s.coins), 480, 15, 2, Hex(0xffe27a), Align::Center);

  const bool full = s.gauge >= 100.f;
  const float gx = 600.f, gw = 130.f;
  g.Text(full ? "A ULT!" : "ULT", gx - 8.f, 15, 2, full ? Hex(0xffe27a) : Hex(0xb8a2dd), Align::Right);
  g.FillRect(gx - 2, 14, gw + 4, 18, Rgb(0, 0, 0, 180));
  const float t = static_cast<float>(w.Frame());
  g.FillRect(gx, 16, gw * s.gauge / 100.f, 14, full ? Hsl(290.f + std::sin(t * 0.2f) * 30.f, 1.f, 0.7f) : Hex(0x7b3fd0));

  g.Text("SCORE " + std::string(6 - std::min<size_t>(6, std::to_string(s.score).size()), '0') + std::to_string(s.score),
         kW - 20.f, 15, 2, Hex(0xffffff), Align::Right);

  if (const Boss* boss = w.GetBoss()) {
    if (!boss->dying) {
      const float bw = 520.f, bx = (kW - bw) * 0.5f, by = 60.f;
      g.Text("VOIDMAW, LORD OF THE VOID", kW * 0.5f, by - 18.f, 2, Hex(0xffd0ff), Align::Center);
      g.FillRect(bx - 3, by, bw + 6, 16, Rgb(0, 0, 0, 180));
      const float ratio = static_cast<float>(boss->Hp()) / static_cast<float>(boss->MaxHp());
      const Color fill[3] = {Hex(0xa24bff), Hex(0xe04bd0), Hex(0xff3c5c)};
      g.FillRect(bx, by + 3, bw * ratio, 10, fill[boss->Phase()]);
      g.FillRect(bx + bw * 0.33f, by, 2, 16, Rgb(255, 255, 255, 128));
      g.FillRect(bx + bw * 0.66f, by, 2, 16, Rgb(255, 255, 255, 128));
      if (boss->AttackNameTimer() > 0 && boss->AttackName()[0] != '\0') {
        g.SetGlobalAlpha(std::min(1.f, static_cast<float>(boss->AttackNameTimer()) / 20.f));
        g.Text(boss->AttackName(), kW * 0.5f, by + 26.f, 2, Hex(0xff9ad0), Align::Center);
        g.SetGlobalAlpha(1.f);
      }
    }
  }

  // 콤보: 오른쪽 위. 오를 때 글자가 커졌다 돌아오고, 남은 시간 막대가 줄어든다.
  if (w.Combo() >= 2) {
    const int combo = w.Combo();
    const float x = kW - 20.f, y = 56.f;
    const float left = static_cast<float>(w.ComboTimer()) / static_cast<float>(World::kComboFrames);
    const Color c = combo >= 10 ? Hsl(std::fmod(t * 6.f, 360.f), 1.f, 0.7f) : combo >= 5 ? Hex(0xffb347) : Hex(0xffe27a);
    g.SetGlobalAlpha(std::min(1.f, left * 4.f));
    g.Text(std::to_string(combo) + " COMBO", x, y, w.ComboPop() > 6 ? 4 : 3, c, Align::Right);
    if (w.ComboMultiplier() > 1) g.Text("SCORE x" + std::to_string(w.ComboMultiplier()), x, y + 32.f, 2, Hex(0xffffff), Align::Right);
    constexpr float bw = 120.f;
    g.FillRect(x - bw, y + 50.f, bw, 4, Rgb(0, 0, 0, 160));
    g.FillRect(x - bw, y + 50.f, std::floor(bw * left / 2.f) * 2.f, 4, c);
    g.SetGlobalAlpha(1.f);
  }

  if (w.BannerTimer() > 0) {
    g.SetGlobalAlpha(std::min(1.f, static_cast<float>(w.BannerTimer()) / 40.f));
    g.Text(w.Banner(), kW * 0.5f, 170, 4, Hex(0xf0dcff), Align::Center);
    if (w.GetBoss() && w.StageIndex() == cfg::kBossStage && w.BannerTimer() > 60) {
      g.Text("THE LORD OF THE VOID APPEARS...", kW * 0.5f, 215, 2, Hex(0xff9ad0), Align::Center);
    }
    g.SetGlobalAlpha(1.f);
  }
}

}  // namespace gengar

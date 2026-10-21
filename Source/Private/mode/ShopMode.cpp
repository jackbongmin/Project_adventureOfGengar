#include "mode/ShopMode.h"

#include <string>

#include "actor/PhantomArt.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "core/Game.h"
#include "data/PlayerStats.h"
#include "data/Upgrades.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "platform/Input.h"

namespace gengar {

void ShopMode::Update(Game& game) {
  ++frame_;
  const std::vector<UpgradeDef>& defs = AllUpgrades();

  if (Debug().bot) {  // 검증용: 60프레임마다 살 수 있는 가장 좋은 업그레이드를 사고, 더 살 게 없으면 출발한다.
    if (frame_ % 60 == 0) {
      static const UpgradeId priority[] = {UpgradeId::Multi, UpgradeId::Power, UpgradeId::Rate, UpgradeId::MaxHp, UpgradeId::Ult, UpgradeId::Jump, UpgradeId::Speed, UpgradeId::Life};
      Session& s = game.Sess();
      bool bought = false;
      for (UpgradeId id : priority) {
        for (const UpgradeDef& def : defs) {
          const int cost = NextCost(def, s);
          if (def.id != id || cost < 0 || s.coins < cost) continue;
          s.coins -= cost;
          ++s.Level(def.id);
          if (def.id == UpgradeId::Life) ++s.lives;
          DebugLog("[shop] bought %s -> Lv.%d (cost %d, coins left %d)", def.name, s.Level(def.id), cost, s.coins);
          bought = true;
          break;
        }
        if (bought) break;
      }
      if (!bought) { DebugLog("[shop] nothing more to buy, departing"); game.LeaveShop(); return; }
    }
    return;
  }

  const int rows = static_cast<int>(defs.size()) + 1;  // 마지막 줄 = 출발
  Input& in = game.In();

  if (in.Pressed(Action::Up)) { selected_ = (selected_ + rows - 1) % rows; game.Snd().Play(Sfx::Select); DebugLog("[shop] cursor up -> %d", selected_); }
  if (in.Pressed(Action::Down)) { selected_ = (selected_ + 1) % rows; game.Snd().Play(Sfx::Select); DebugLog("[shop] cursor down -> %d", selected_); }
  if (errorTimer_ > 0) --errorTimer_;

  if (in.Pressed(Action::Confirm)) {
    DebugLog("[shop] confirm on row %d (coins %d)", selected_, game.Sess().coins);
    if (selected_ == rows - 1) { game.LeaveShop(); return; }
    const UpgradeDef& def = defs[static_cast<size_t>(selected_)];
    Session& s = game.Sess();
    const int cost = NextCost(def, s);
    if (cost >= 0 && s.coins >= cost) {
      s.coins -= cost;
      ++s.Level(def.id);
      if (def.id == UpgradeId::Life) ++s.lives;
      game.Snd().Play(Sfx::Buy);
      errorTimer_ = 0;
    } else {
      errorTimer_ = 40;
      game.Snd().Play(Sfx::Error);
    }
  }
}

void ShopMode::Render(Graphics& g) {
  constexpr float W = static_cast<float>(cfg::kScreenW), H = static_cast<float>(cfg::kScreenH);
  const Session& s = game_.Sess();
  const std::vector<UpgradeDef>& defs = AllUpgrades();

  for (float y = 0; y < H; y += 8.f) g.FillRect(0, y, W, 8.f, Lerp(Hex(0x12062a), Hex(0x2d0f4f), y / H));
  g.Text("SOUL SHOP", 50, 36, 5, Hex(0xd9b8ff));
  g.Text("$ " + std::to_string(s.coins), W - 50, 40, 4, Hex(0xffe27a), Align::Right);
  g.Text("UP/DOWN SELECT   ENTER/Z BUY", 50, 82, 2, Hex(0x9a82c0));

  for (size_t i = 0; i < defs.size() + 1; ++i) {
    const float y = 108.f + static_cast<float>(i) * 44.f;
    const bool sel = static_cast<int>(i) == selected_;
    g.FillRect(40, y, 590, 38, sel ? Rgb(160, 90, 255, 90) : Rgb(255, 255, 255, 15));
    if (sel) {
      g.FillRect(40, y, 590, 2, Hex(0xc58bff));
      g.FillRect(40, y + 36, 590, 2, Hex(0xc58bff));
    }
    if (i == defs.size()) {
      g.Text("DEPART TO NEXT STAGE >", 60, y + 11, 2, sel ? Hex(0xffe27a) : Hex(0xffffff));
      continue;
    }
    const UpgradeDef& def = defs[i];
    const int level = s.Level(def.id), cost = NextCost(def, s);
    g.Text(def.name, 56, y + 6, 2, Hex(0xffffff));
    g.Text(def.desc, 56, y + 24, 1, Hex(0xb8a2dd), Align::Left, false);
    for (int k = 0; k < def.maxLevel; ++k) {
      g.FillRect(420.f + static_cast<float>(k) * 14.f, y + 14, 10, 10, k < level ? Hex(0xc58bff) : Rgb(255, 255, 255, 46));
    }
    const bool can = cost >= 0 && s.coins >= cost;
    g.Text(cost < 0 ? "MAX" : "$ " + std::to_string(cost), 616, y + 11, 2, cost < 0 ? Hex(0x9a82c0) : can ? Hex(0xffe27a) : Hex(0xff6a8a), Align::Right);
  }
  if (errorTimer_ > 0) g.Text("NOT ENOUGH COINS OR ALREADY MAX", 335, 520, 2, Hex(0xff6a8a), Align::Center);

  // 현재 능력치 패널
  g.FillRect(660, 108, 260, 330, Rgb(255, 255, 255, 18));
  DrawPhantom(g, {790.f, 180.f}, 1, static_cast<float>(frame_), 2.f);
  const std::string lines[] = {
      "ATTACK    " + std::to_string(static_cast<int>(stats::Damage(s))),
      "SHOTS     " + std::to_string(stats::ShotCount(s)),
      "FIRE DELAY " + std::to_string(stats::FireCooldown(s)) + "F",
      "MAX HEART " + std::to_string(stats::MaxHp(s)),
      "JUMP      LV." + std::to_string(s.Level(UpgradeId::Jump)),
      "SPEED     LV." + std::to_string(s.Level(UpgradeId::Speed)),
      "ULT       LV." + std::to_string(s.Level(UpgradeId::Ult)),
      "LIVES     " + std::to_string(s.lives),
  };
  for (size_t i = 0; i < 8; ++i) g.Text(lines[i], 684, 244.f + static_cast<float>(i) * 22.f, 2, Hex(0xe6d6ff), Align::Left, false);
  const bool boss = s.stage + 1 == cfg::kBossStage;
  g.Text(boss ? "NEXT: FINAL BOSS!" : "NEXT: STAGE " + std::to_string(s.stage + 2), W - 50.f, 84, 2, boss ? Hex(0xff7aa8) : Hex(0xd9c2ff), Align::Right);

  // 선택한 항목을 사면 무엇이 얼마로 바뀌는지 (패널 아래)
  if (selected_ < static_cast<int>(defs.size())) {
    const UpgradeDef& sel = defs[static_cast<size_t>(selected_)];
    if (NextCost(sel, s) < 0) {
      g.Text("MAXED OUT", 790, 466, 3, Hex(0x9a82c0), Align::Center);
    } else {
      const UpgradePreview pv = PreviewUpgrade(sel, s);
      g.Text("IF YOU BUY:", 790, 448, 2, Hex(0x9a82c0), Align::Center, false);
      g.Text(pv.label, 790, 470, 2, Hex(0xe6d6ff), Align::Center, false);
      g.Text(pv.change, 790, 494, 3, Hex(0xffe27a), Align::Center);
    }
  } else {
    g.Text("READY?", 790, 470, 3, Hex(0xffe27a), Align::Center);
  }
}

}  // namespace gengar

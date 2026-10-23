#include "mode/ShopMode.h"

#include <cmath>
#include <string>

#include "actor/PhantomArt.h"
#include "art/Art.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "core/Game.h"
#include "data/PlayerStats.h"
#include "data/Upgrades.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "platform/Input.h"
#include "ui/Menu.h"

namespace gengar {
namespace {

// 모서리를 도트 한 칸 깎은 상자: 테두리(edge) 2px + 안쪽(fill)
void Box(Graphics& g, float x, float y, float w, float h, Color fill, Color edge) {
  g.FillRect(x + 2.f, y, w - 4.f, h, edge);
  g.FillRect(x, y + 2.f, w, h - 4.f, edge);
  g.FillRect(x + 2.f, y + 2.f, w - 4.f, h - 4.f, fill);
}

// 레벨 칸 하나: 산 단계는 밝은 보라, 남은 단계는 테두리만 보이는 빈 칸
void Pip(Graphics& g, float x, float y, float size, bool on, bool flash) {
  Box(g, x, y, size, size, on || flash ? Hex(0x12061f) : Hex(0x1c0a34), on || flash ? Hex(0x12061f) : Hex(0x6a4a9a));
  if (!on && !flash) return;
  g.FillRect(x + 2.f, y + 2.f, size - 4.f, size - 4.f, flash ? Hex(0xffffff) : Hex(0xc58bff));
  if (!flash) g.FillRect(x + 2.f, y + 2.f, size - 4.f, 2.f, Hex(0xe6ccff));
}

// 업그레이드 분류: 목록 왼쪽 색 띠와 상세 패널의 이름표
struct Category {
  const char* name;
  Color color;
};
Category CategoryOf(UpgradeId id) {
  switch (id) {
    case UpgradeId::Power: case UpgradeId::Rate: case UpgradeId::Multi: return {"ATTACK", Hex(0xff6a6a)};
    case UpgradeId::MaxHp: case UpgradeId::DoubleJump: case UpgradeId::Speed: return {"BODY", Hex(0x6fe08a)};
    default: return {"SPECIAL", Hex(0xff7ab8)};
  }
}

}  // namespace

void ShopMode::Update(Game& game) {
  ++frame_;
  const std::vector<UpgradeDef>& defs = AllUpgrades();

  if (Debug().bot) {  // 검증용: 60프레임마다 살 수 있는 가장 좋은 업그레이드를 사고, 더 살 게 없으면 출발한다.
    if (frame_ % 60 == 0) {
      static const UpgradeId priority[] = {UpgradeId::Multi, UpgradeId::Power, UpgradeId::Rate, UpgradeId::MaxHp, UpgradeId::Ult, UpgradeId::DoubleJump, UpgradeId::Speed, UpgradeId::Life};
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
  if (flashTimer_ > 0) --flashTimer_;

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
      flashRow_ = selected_;
      flashTimer_ = 24;
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
  const int rows = static_cast<int>(defs.size()) + 1;
  const float t = static_cast<float>(frame_);

  for (float y = 0; y < H; y += 8.f) g.FillRect(0, y, W, 8.f, Lerp(Hex(0x12062a), Hex(0x2d0f4f), y / H));

  // ---- 머리: 제목, 다음 목적지, 가진 코인 ----
  g.Text("SOUL SHOP", 40.f, 26.f, 5, Hex(0xd9b8ff));
  const bool boss = s.stage + 1 == cfg::kBossStage;
  const std::string next = boss ? "FINAL BOSS" : "STAGE " + std::to_string(s.stage + 2);
  g.Text("NEXT", 40.f, 72.f, 2, Hex(0x9a82c0));
  g.Text(next, 96.f, 72.f, 2, boss ? Hex(0xff7aa8) : Hex(0xe6d6ff));
  {
    const float w = static_cast<float>(g.TextWidth(std::to_string(s.coins), 4)) + 32.f + 8.f + 36.f;
    Box(g, W - 32.f - w, 20.f, w, 50.f, Hex(0x0e0420), Hex(0x5a2a9a));
    DrawCoinAmount(g, s.coins, W - 50.f, 31.f, 4, Hex(0xffe27a), Align::Right);
  }

  // ---- 왼쪽: 업그레이드 목록 ----
  constexpr float kListX = 28.f, kListY = 96.f, kListW = 580.f, kRowH = 40.f, kRowGap = 4.f;
  for (int i = 0; i < rows; ++i) {
    const float y = kListY + static_cast<float>(i) * (kRowH + kRowGap);
    const bool sel = i == selected_;
    const bool depart = i == rows - 1;
    const UpgradeDef* def = depart ? nullptr : &defs[static_cast<size_t>(i)];
    const int level = def ? s.Level(def->id) : 0;
    const int cost = def ? NextCost(*def, s) : 0;
    const bool maxed = def && cost < 0;
    const bool can = def && !maxed && s.coins >= cost;

    if (sel) Box(g, kListX, y, kListW, kRowH, Hex(0x3a1a66), Hex(0xc58bff));
    else Box(g, kListX, y, kListW, kRowH, depart ? Hex(0x2a1048) : Hex(0x1c0a34), Hex(0x2e1452));
    if (flashTimer_ > 0 && i == flashRow_) g.FillRect(kListX + 4.f, y + 4.f, kListW - 8.f, kRowH - 8.f, Rgb(255, 230, 140, flashTimer_ * 6));

    // 분류 색 띠 + 아이콘 칸
    if (!depart) g.FillRect(kListX + 2.f, y + 4.f, 4.f, kRowH - 8.f, maxed ? Hex(0x5a4a7a) : CategoryOf(def->id).color);
    Box(g, kListX + 10.f, y + 4.f, 32.f, 32.f, Hex(0x12061f), Hex(0x12061f));
    const SpriteId icon = depart ? art::A().departIcon : art::A().upgradeIcon[static_cast<int>(def->id)];
    const float bob = sel ? std::floor(std::sin(t * 0.15f) * 1.5f) : 0.f;
    g.DrawSprite(icon, kListX + 14.f, y + 8.f + bob, 2, false, maxed ? 0.45f : 1.f);

    if (depart) {
      g.Text("DEPART TO " + next, kListX + 54.f, y + 13.f, 2, sel ? Hex(0xffe27a) : Hex(0xffffff));
      continue;
    }
    const Color nameColor = maxed ? Hex(0x8a74b0) : sel ? Hex(0xffe27a) : Hex(0xffffff);
    g.Text(def->name, kListX + 54.f, y + 13.f, 2, nameColor);

    // 레벨 칸: 채운 칸 = 산 단계. 방금 산 칸은 하얗게 반짝인다.
    for (int k = 0; k < def->maxLevel; ++k) {
      const bool justBought = flashTimer_ > 0 && i == flashRow_ && k == level - 1;
      Pip(g, kListX + 330.f + static_cast<float>(k) * 18.f, y + 13.f, 14.f, k < level, justBought);
    }

    // 가격 (모자라면 빨강) / MAX
    const float right = kListX + kListW - 14.f;
    if (maxed) {
      Box(g, right - 56.f, y + 9.f, 56.f, 22.f, Hex(0x2e1452), Hex(0x5a2a9a));
      g.Text("MAX", right - 28.f, y + 13.f, 2, Hex(0xc9b0ee), Align::Center);
    } else {
      DrawCoinAmount(g, cost, right, y + 13.f, 2, can ? Hex(0xffe27a) : Hex(0xff6a8a), Align::Right);
    }
  }

  // ---- 오른쪽: 선택한 항목 자세히 ----
  constexpr float kPanelX = 628.f, kPanelY = 96.f, kPanelW = 304.f, kPanelH = 392.f;
  constexpr float cx = kPanelX + kPanelW * 0.5f;
  DrawPixelPanel(g, kPanelX, kPanelY, kPanelW, kPanelH);

  if (selected_ == rows - 1) {
    DrawPhantom(g, {cx, kPanelY + 92.f}, 1, t, 2.4f);
    g.Text("NEXT", cx, kPanelY + 170.f, 2, Hex(0x9a82c0), Align::Center);
    g.Text(next, cx, kPanelY + 194.f, 4, boss ? Hex(0xff7aa8) : Hex(0xffe27a), Align::Center);
    // 지금 상태 요약: 최대 하트와 목숨
    const int hearts = stats::MaxHp(s);
    const float hx = cx - static_cast<float>(hearts) * 11.f;
    for (int k = 0; k < hearts; ++k) g.DrawSprite(art::A().heartFull, hx + static_cast<float>(k) * 22.f, kPanelY + 250.f, 2);
    g.Text("LIFE x" + std::to_string(s.lives), cx, kPanelY + 280.f, 2, Hex(0xe6d6ff), Align::Center);
    Box(g, kPanelX + 24.f, kPanelY + 322.f, kPanelW - 48.f, 46.f, Hex(0x2a1048), Hex(0xc58bff));
    const float capW = DrawKeyCap(g, "ENTER", cx - 66.f, kPanelY + 332.f);
    g.Text("GO!", cx - 66.f + capW + 10.f, kPanelY + 338.f, 2, Hex(0xffe27a));
  } else {
    const UpgradeDef& def = defs[static_cast<size_t>(selected_)];
    const int level = s.Level(def.id), cost = NextCost(def, s);
    const bool maxed = cost < 0, can = !maxed && s.coins >= cost;

    // 큰 아이콘 (방금 샀으면 통통 튄다)
    const float pop = flashTimer_ > 0 && flashRow_ == selected_ ? std::fabs(std::sin(static_cast<float>(flashTimer_) * 0.35f)) * 6.f : 0.f;
    Box(g, cx - 38.f, kPanelY + 22.f, 76.f, 76.f, Hex(0x12061f), Hex(0x5a2a9a));
    g.DrawSprite(art::A().upgradeIcon[static_cast<int>(def.id)], cx - 30.f, kPanelY + 30.f - pop, 5);
    // 분류 이름표 (왼쪽 위)
    const Category cat = CategoryOf(def.id);
    const float tagW = static_cast<float>(g.TextWidth(cat.name, 2)) + 12.f;
    Box(g, kPanelX + 18.f, kPanelY + 18.f, tagW, 22.f, Hex(0x12061f), cat.color);
    g.Text(cat.name, kPanelX + 24.f, kPanelY + 22.f, 2, cat.color, Align::Left, false);

    g.Text(def.name, cx, kPanelY + 112.f, 2, Hex(0xffffff), Align::Center);
    // 설명: 쉼표마다 줄을 나눈다
    std::string desc = def.desc;
    float dy = kPanelY + 136.f;
    for (size_t cut; (cut = desc.find(", ")) != std::string::npos; desc = desc.substr(cut + 2), dy += 20.f) {
      g.Text(desc.substr(0, cut), cx, dy, 2, Hex(0xb8a2dd), Align::Center, false);
    }
    g.Text(desc, cx, dy, 2, Hex(0xb8a2dd), Align::Center, false);

    // 레벨
    const float pipsW = static_cast<float>(def.maxLevel) * 22.f - 6.f;
    for (int k = 0; k < def.maxLevel; ++k) Pip(g, cx - pipsW * 0.5f + static_cast<float>(k) * 22.f, kPanelY + 186.f, 16.f, k < level, false);
    g.Text("LV " + std::to_string(level) + " / " + std::to_string(def.maxLevel), cx, kPanelY + 210.f, 2, Hex(0x9a82c0), Align::Center);

    // 사면 무엇이 얼마로 바뀌는지: 지금 값(흰색) > 바뀐 값(초록)
    for (float x = kPanelX + 24.f; x < kPanelX + kPanelW - 24.f; x += 12.f) g.FillRect(x, kPanelY + 238.f, 4.f, 4.f, Hex(0x5a2a9a));
    if (maxed) {
      g.Text("FULLY UPGRADED", cx, kPanelY + 272.f, 2, Hex(0xc9b0ee), Align::Center);
    } else {
      const UpgradePreview pv = PreviewUpgrade(def, s);
      g.Text(pv.label, cx, kPanelY + 254.f, 2, Hex(0x9a82c0), Align::Center, false);
      const size_t arrow = pv.change.find(" -> ");
      const std::string from = pv.change.substr(0, arrow), to = arrow == std::string::npos ? "" : pv.change.substr(arrow + 4);
      const float wFrom = static_cast<float>(g.TextWidth(from, 4)), wArrow = static_cast<float>(g.TextWidth(">", 4)), wTo = static_cast<float>(g.TextWidth(to, 4));
      float x = cx - (wFrom + wArrow + wTo + 32.f) * 0.5f;
      g.Text(from, x, kPanelY + 278.f, 4, Hex(0xffffff));
      x += wFrom + 16.f;
      g.Text(">", x, kPanelY + 278.f, 4, Hex(0x9a5cff));
      x += wArrow + 16.f;
      g.Text(to, x, kPanelY + 278.f, 4, Hex(0x6fe08a));
    }

    // 구매 칸: 살 수 있음 / 코인 부족(누르면 흔들림) / 최대
    const float shake = errorTimer_ > 0 ? std::sin(static_cast<float>(errorTimer_) * 1.3f) * static_cast<float>(errorTimer_) * 0.15f : 0.f;
    const float bx = kPanelX + 24.f + shake, by = kPanelY + 322.f, bw = kPanelW - 48.f;
    if (maxed) {
      Box(g, bx, by, bw, 46.f, Hex(0x2a1048), Hex(0x5a2a9a));
      g.Text("MAXED OUT", cx, by + 16.f, 2, Hex(0xc9b0ee), Align::Center);
    } else if (can) {
      Box(g, bx, by, bw, 46.f, Hex(0x2a1048), Hex(0xffe27a));
      const float capW = DrawKeyCap(g, "ENTER", bx + 16.f, by + 10.f);
      g.Text("BUY", bx + 16.f + capW + 10.f, by + 16.f, 2, Hex(0xffffff));
      DrawCoinAmount(g, cost, bx + bw - 16.f, by + 16.f, 2, Hex(0xffe27a), Align::Right);
    } else {
      Box(g, bx, by, bw, 46.f, Hex(0x2a0a1e), Hex(0xff6a8a));
      g.Text("NEED", bx + 16.f, by + 16.f, 2, Hex(0xff9ab4));
      DrawCoinAmount(g, cost - s.coins, bx + 16.f + static_cast<float>(g.TextWidth("NEED ", 2)), by + 16.f, 2, Hex(0xff6a8a));
      g.Text("MORE", bx + bw - 16.f, by + 16.f, 2, Hex(0xff9ab4), Align::Right);
    }
  }

  // ---- 아래: 키 안내 ----
  DrawKeyHints(g, {{{"UP", "DOWN"}, "SELECT"}, {{"ENTER"}, selected_ == rows - 1 ? "DEPART" : "BUY"}}, W * 0.5f, 504.f);
}

}  // namespace gengar

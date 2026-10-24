#include "mode/TitleMode.h"

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <string>

#include "actor/PhantomArt.h"
#include "art/Art.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "core/Game.h"
#include "platform/Graphics.h"
#include "platform/Input.h"
#include "ui/Menu.h"

namespace gengar {
namespace {

constexpr float W = static_cast<float>(cfg::kScreenW), H = static_cast<float>(cfg::kScreenH);

struct Item {
  const char* label;
  const char* hint;
};

// 메인 메뉴 항목. 순서가 곧 화면 순서이며 UpdateMain 의 switch 와 맞춰야 한다.
enum MainItem { kNewGame, kBoss, kStages, kControls, kSettings, kGallery, kQuit, kMainCount };
const Item kMain[kMainCount] = {
    {"NEW GAME", "START FROM THE OPENING"},
    {"BOSS BATTLE", "SHOP WITH 150 COINS, THEN FIGHT VOIDMAW"},
    {"STAGE SELECT", "PRACTICE A STAGE WITH ITS USUAL UPGRADES"},
    {"CONTROLS", "KEYS AND HOW TO PLAY"},
    {"SETTINGS", "VOLUME, SCREEN SHAKE, JUMP"},
    {"SPRITE GALLERY", "BROWSE ALL PIXEL SPRITES"},
    {"QUIT", "CLOSE THE GAME"},
};

#ifdef __EMSCRIPTEN__
constexpr int kMainShown = kMainCount - 1;  // 웹에서는 종료 버튼이 의미 없다
#else
constexpr int kMainShown = kMainCount;
#endif

int Wrap(int v, int n) { return (v % n + n) % n; }

// 조작법: 키를 키캡으로 그리고 이동 / 행동 / 메뉴로 묶는다.
struct ControlRow {
  std::initializer_list<const char*> keys;
  const char* action;
  const char* note;  // 옅은 색 보충 설명 (없으면 "")
};
struct ControlGroup {
  const char* name;
  Color color;
  std::initializer_list<ControlRow> rows;
};

void DrawControls(Graphics& g) {
  const ControlGroup groups[] = {
      {"MOVE", Hex(0x6fe08a), {{{"LEFT", "RIGHT"}, "MOVE", ""}, {{"C", "SPACE", "UP"}, "JUMP", "HOLD = HIGHER"}}},
      {"ACTION", Hex(0xff6a6a), {{{"Z"}, "SHADOW BALL", "HOLD = REPEAT"}, {{"X", "SHIFT"}, "DASH", "QUICK BURST"}, {{"A"}, "ULTIMATE", "WHEN GAUGE IS FULL"}}},
      {"MENU", Hex(0xc58bff), {{{"ESC", "P"}, "PAUSE", ""}, {{"ENTER"}, "SELECT", ""}, {{"TAB"}, "STATUS", "STATS AND CARDS"}}},
  };
  constexpr float kPanelX = 110.f, kPanelY = 112.f, kPanelW = 740.f, kKeysRight = 400.f;
  DrawPixelPanel(g, kPanelX, kPanelY, kPanelW, 340.f);
  float y = kPanelY + 22.f;
  for (const ControlGroup& grp : groups) {
    // 분류 이름 + 색 띠
    g.FillRect(kPanelX + 24.f, y + 2.f, 4.f, 14.f, grp.color);
    g.Text(grp.name, kPanelX + 34.f, y + 2.f, 2, grp.color, Align::Left, false);
    for (float x = kPanelX + 34.f + static_cast<float>(g.TextWidth(grp.name, 2)) + 10.f; x < kPanelX + kPanelW - 24.f; x += 12.f) g.FillRect(x, y + 8.f, 4.f, 2.f, Hex(0x3e1d6a));
    y += 24.f;
    for (const ControlRow& row : grp.rows) {
      float w = 0.f;  // 키캡들을 오른쪽 정렬하려고 폭을 먼저 잰다
      for (const char* k : row.keys) {
        const std::string s = k;
        w += (s == "UP" || s == "DOWN" || s == "LEFT" || s == "RIGHT" ? 26.f : std::max(26.f, static_cast<float>(g.TextWidth(s, 2)) + 14.f)) + 6.f;
      }
      float x = kKeysRight - w;
      for (const char* k : row.keys) x += DrawKeyCap(g, k, x, y) + 6.f;
      g.Text(row.action, kKeysRight + 18.f, y + 6.f, 2, Hex(0xffffff));
      if (row.note[0] != '\0') g.Text(row.note, kKeysRight + 18.f + static_cast<float>(g.TextWidth(row.action, 2)) + 16.f, y + 6.f, 2, Hex(0x9a82c0), Align::Left, false);
      y += 32.f;
    }
    y += 6.f;
  }
}

}  // namespace

void TitleMode::Update(Game& game) {
  ++frame_;
  if ((Debug().bot || Debug().stompDemo > 0 || Debug().startPicks > 0 || Debug().startInShop || !Debug().cards.empty()) && frame_ > 30) { game.StartNewRun(); return; }  // bot=1 / stompdemo / pick / shop / cards 이면 시네마틱 없이 자동 시작
  switch (screen_) {
    case Screen::Main: UpdateMain(game); break;
    case Screen::Stages: UpdateStages(game); break;
    case Screen::Settings: UpdateSettings(game); break;
    case Screen::Controls:
      if (game.In().Pressed(Action::Confirm) || game.In().Pressed(Action::Pause)) { game.Snd().Play(Sfx::Select); Go(Screen::Main); cursor_ = kControls; }
      break;
  }
}

void TitleMode::UpdateMain(Game& game) {
  Input& in = game.In();
  if (in.Pressed(Action::Gallery)) { game.GoGallery(); return; }
  if (in.Pressed(Action::Up)) { cursor_ = Wrap(cursor_ - 1, kMainShown); game.Snd().Play(Sfx::Select); }
  if (in.Pressed(Action::Down)) { cursor_ = Wrap(cursor_ + 1, kMainShown); game.Snd().Play(Sfx::Select); }
  if (!in.Pressed(Action::Confirm)) return;
  game.Snd().Play(Sfx::Select);
  switch (cursor_) {
    case kNewGame: game.GoIntro(); break;
    case kBoss: game.StartBossFromShop(); break;
    case kStages: Go(Screen::Stages); break;
    case kControls: Go(Screen::Controls); break;
    case kSettings: Go(Screen::Settings); break;
    case kGallery: game.GoGallery(); break;
    case kQuit: game.Quit(); break;
  }
}

void TitleMode::UpdateStages(Game& game) {
  Input& in = game.In();
  constexpr int kRows = cfg::kBossStage + 1;  // 보스 직전까지의 스테이지들 + BACK
  if (in.Pressed(Action::Pause)) { game.Snd().Play(Sfx::Select); Go(Screen::Main); cursor_ = kStages; return; }
  if (in.Pressed(Action::Up)) { cursor_ = Wrap(cursor_ - 1, kRows); game.Snd().Play(Sfx::Select); }
  if (in.Pressed(Action::Down)) { cursor_ = Wrap(cursor_ + 1, kRows); game.Snd().Play(Sfx::Select); }
  if (!in.Pressed(Action::Confirm)) return;
  game.Snd().Play(Sfx::Select);
  if (cursor_ == kRows - 1) { Go(Screen::Main); cursor_ = kStages; }
  else game.StartAtStage(cursor_);
}

void TitleMode::UpdateSettings(Game& game) {
  if (UpdateSettingsMenu(game, cursor_)) { Go(Screen::Main); cursor_ = kSettings; }
}

void TitleMode::Render(Graphics& g) {
  for (float y = 0; y < H; y += 8.f) g.FillRect(0, y, W, 8.f, Lerp(Hex(0x0e0420), Hex(0x3a1260), y / H));
  for (int i = 0; i < 80; ++i) {
    const float a = 0.3f + 0.5f * std::sin(static_cast<float>(frame_) * 0.04f + static_cast<float>(i));
    g.FillRect(static_cast<float>((i * 151) % cfg::kScreenW), static_cast<float>((i * 67) % 400), 2, 2, Rgb(255, 255, 255, static_cast<int>(std::max(0.f, a) * 255)));
  }

  // 배경: 풀밭 땅 위를 되찾은 보물 모자를 쓰고 신나서 오가는 팬텀 (불투명. 글자는 뒤에 그려지는 팬텀 위에 덮인다)
  {
    const art::TileSet& ts = art::A().tiles[0];  // 1스테이지(풀밭) 타일
    const float groundY = static_cast<float>(cfg::kGroundRow * cfg::kTile);
    for (float x = 0; x < W; x += static_cast<float>(cfg::kTile)) {
      g.DrawSprite(ts.groundTop, x, groundY, art::kPixel);
      for (float y = groundY + static_cast<float>(cfg::kTile); y < H; y += static_cast<float>(cfg::kTile)) g.DrawSprite(ts.dirt, x, y, art::kPixel);
    }
    const float t = static_cast<float>(frame_);
    const float x = W * 0.5f + std::sin(t * 0.012f) * (W * 0.38f);
    const int facing = std::cos(t * 0.012f) >= 0.f ? 1 : -1;
    const float hop = std::fabs(std::sin(t * 0.09f)) * 26.f;  // 콩콩 뛰며 돌아다닌다
    PhantomStyle style;
    style.happy = true;
    style.hat = true;
    style.hatGlow = 0.6f + 0.4f * std::sin(t * 0.1f);
    style.squash = {1.f + 0.06f * std::cos(t * 0.18f), 1.f - 0.06f * std::cos(t * 0.18f)};
    DrawPhantom(g, {std::floor(x), groundY - 56.f - std::floor(hop)}, facing, t, 3.6f, 1.f, style);
  }

  if (screen_ == Screen::Main) {
    // 제목은 버튼들 위에서 둥실둥실 떠다닌다
    const float tf = static_cast<float>(frame_);
    const float ty = 118.f + std::floor(std::sin(tf * 0.05f) * 9.f);
    g.Text("ADVENTURE OF GENGAR", W * 0.5f + 4.f, ty + 5.f, 7, Rgb(0, 0, 0, 120), Align::Center);  // 그림자
    g.Text("ADVENTURE OF GENGAR", W * 0.5f, ty, 7, Hex(0xd9b8ff), Align::Center);
    g.Text("THE LORD OF THE VOID AWAITS", W * 0.5f, ty + 66.f + std::floor(std::sin(tf * 0.05f + 0.8f) * 3.f), 2, Hex(0xffffff), Align::Center);
    for (int i = 0; i < kMainShown; ++i) DrawMenuRow(g, kMain[i].label, W * 0.5f, 262.f + static_cast<float>(i) * 34.f, i == cursor_, frame_);
    g.Text(kMain[cursor_].hint, W * 0.5f, 508, 2, Hex(0x9a82c0), Align::Center);
    return;
  }

  g.Text(screen_ == Screen::Stages ? "STAGE SELECT" : screen_ == Screen::Controls ? "CONTROLS" : "SETTINGS", W * 0.5f, 60, 6, Hex(0xd9b8ff), Align::Center);

  if (screen_ == Screen::Stages) {
    for (int i = 0; i < cfg::kBossStage; ++i) DrawMenuRow(g, "STAGE " + std::to_string(i + 1), W * 0.5f, 190.f + static_cast<float>(i) * 50.f, i == cursor_, frame_);
    DrawMenuRow(g, "BACK", W * 0.5f, 190.f + static_cast<float>(cfg::kBossStage) * 50.f, cursor_ == cfg::kBossStage, frame_);
    g.Text("YOU START WITH THE UPGRADES YOU WOULD NORMALLY HAVE BOUGHT BY THEN", W * 0.5f, 466, 2, Hex(0x9a82c0), Align::Center);
    DrawKeyHints(g, {{{"UP", "DOWN"}, "SELECT"}, {{"ENTER"}, "START"}, {{"ESC"}, "BACK"}}, W * 0.5f, 500.f);
  } else if (screen_ == Screen::Controls) {
    DrawControls(g);
    g.Text("COLLECT COINS, BUY UPGRADES IN THE SHOP, BEAT THE VOID LORD", W * 0.5f, 470, 2, Hex(0x9a82c0), Align::Center);
    DrawKeyHints(g, {{{"ENTER"}, "BACK"}}, W * 0.5f, 500.f);
  } else {
    DrawPixelPanel(g, 150.f, 116.f, 660.f, 300.f);
    DrawSettingsMenu(g, cursor_, frame_, 174.f, 138.f, 612.f, 382.f);
    DrawKeyHints(g, {{{"UP", "DOWN"}, "SELECT"}, {{"LEFT", "RIGHT"}, "CHANGE"}, {{"ESC"}, "BACK"}}, W * 0.5f, 500.f);
  }
}

}  // namespace gengar

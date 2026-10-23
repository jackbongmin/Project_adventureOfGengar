// 일시정지 메뉴: RESUME / RESTART / SETTINGS / MAIN MENU.
// 다시 시작과 메인 메뉴는 진행이 사라지므로 한 번 더 물어본다.
#include <cmath>
#include <string>

#include "actor/PhantomArt.h"
#include "core/Config.h"
#include "core/Game.h"
#include "core/Session.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "platform/Input.h"
#include "state/play/PlayPhases.h"
#include "ui/Menu.h"

namespace gengar {
namespace {

constexpr float kW = static_cast<float>(cfg::kScreenW);
constexpr float kH = static_cast<float>(cfg::kScreenH);
constexpr float kCx = kW * 0.5f;

enum PauseItem { kResume, kRestart, kSettings, kMainMenu, kPauseCount };
struct Item {
  const char* label;
  const char* hint;
};
const Item kItems[kPauseCount] = {
    {"RESUME", "BACK TO THE ADVENTURE"},
    {"RESTART", "TRY THIS STAGE AGAIN FROM THE START"},
    {"SETTINGS", "VOLUME, SCREEN SHAKE, JUMP"},
    {"MAIN MENU", "END THIS RUN AND RETURN TO THE TITLE"},
};

int Wrap(int v, int n) { return (v % n + n) % n; }

class PausedPhase : public PlayPhase {
 public:
  const char* Name() const override { return "Paused"; }
  void Enter(PlayMode& m) override { m.GetGame().Snd().Play(Sfx::Select); }

  void Update(PlayMode& m) override {
    ++frame_;
    switch (screen_) {
      case Screen::Main: UpdateMain(m); break;
      case Screen::Settings:
        if (UpdateSettingsMenu(m.GetGame(), cursor_)) Go(Screen::Main, kSettings);
        break;
      case Screen::ConfirmRestart:
      case Screen::ConfirmQuit: UpdateConfirm(m); break;
    }
  }

  void Render(PlayMode& m, Graphics& g) override {
    // 멈춘 화면 위에 보랏빛 막 + 브라운관 주사선
    g.FillRect(0, 0, kW, kH, Rgb(10, 0, 25, 150));
    for (float y = 0; y < kH; y += 4.f) g.FillRect(0, y, kW, 2.f, Rgb(0, 0, 0, 40));

    switch (screen_) {
      case Screen::Main: RenderMain(m, g); break;
      case Screen::Settings: RenderSettings(g); break;
      case Screen::ConfirmRestart:
      case Screen::ConfirmQuit: RenderConfirm(g); break;
    }
  }

 private:
  enum class Screen { Main, Settings, ConfirmRestart, ConfirmQuit };

  void Go(Screen s, int cursor) { screen_ = s; cursor_ = cursor; }

  void UpdateMain(PlayMode& m) {
    Game& game = m.GetGame();
    Input& in = game.In();
    if (in.Pressed(Action::Pause)) { game.Snd().Play(Sfx::Select); m.Phase().Change(MakePlayingPhase()); return; }
    if (in.Pressed(Action::Up)) { cursor_ = Wrap(cursor_ - 1, kPauseCount); game.Snd().Play(Sfx::Select); }
    if (in.Pressed(Action::Down)) { cursor_ = Wrap(cursor_ + 1, kPauseCount); game.Snd().Play(Sfx::Select); }
    if (!in.Pressed(Action::Confirm)) return;
    game.Snd().Play(Sfx::Select);
    switch (cursor_) {
      case kResume: m.Phase().Change(MakePlayingPhase()); break;
      case kRestart: Go(Screen::ConfirmRestart, 1); break;  // 실수로 누르지 않도록 NO 에서 시작
      case kSettings: Go(Screen::Settings, 0); break;
      case kMainMenu: Go(Screen::ConfirmQuit, 1); break;
    }
  }

  // 커서 0 = YES, 1 = NO
  void UpdateConfirm(PlayMode& m) {
    Game& game = m.GetGame();
    Input& in = game.In();
    const int back = screen_ == Screen::ConfirmRestart ? kRestart : kMainMenu;
    if (in.Pressed(Action::Pause)) { game.Snd().Play(Sfx::Select); Go(Screen::Main, back); return; }
    if (in.Pressed(Action::Left) || in.Pressed(Action::Right)) { cursor_ = 1 - cursor_; game.Snd().Play(Sfx::Select); }
    if (!in.Pressed(Action::Confirm)) return;
    game.Snd().Play(Sfx::Select);
    if (cursor_ == 1) { Go(Screen::Main, back); return; }
    if (screen_ == Screen::ConfirmRestart) game.RestartStage();
    else game.GoTitle();
  }

  // 패널 위 가운데에 떠 있는 제목. 그림자를 깔고 둥실거린다.
  void Title(Graphics& g, const char* text, float y, int scale) const {
    const float bob = std::floor(std::sin(static_cast<float>(frame_) * 0.06f) * 3.f);
    g.Text(text, kCx + 4.f, y + bob + 4.f, scale, Rgb(0, 0, 0, 140), Align::Center);
    g.Text(text, kCx, y + bob, scale, Hex(0xd9b8ff), Align::Center);
  }

  // 도트를 띄엄띄엄 찍은 구분선
  static void Divider(Graphics& g, float y, float w) {
    for (float x = kCx - w * 0.5f; x < kCx + w * 0.5f; x += 12.f) g.FillRect(x, y, 4.f, 4.f, Hex(0x5a2a9a));
  }

  void RenderMain(PlayMode& m, Graphics& g) const {
    const float px = 250.f, py = 104.f, pw = 460.f, ph = 352.f;
    DrawPixelPanel(g, px, py, pw, ph);

    // 패널 모서리에 걸터앉아 쉬는 팬텀
    const float t = static_cast<float>(frame_);
    DrawPhantom(g, {px + pw - 34.f, py - 18.f + std::floor(std::sin(t * 0.07f) * 3.f)}, -1, t, 1.6f);

    Title(g, "PAUSED", py + 30.f, 6);
    const Session& s = m.GetGame().Sess();
    const std::string where = s.stage == cfg::kBossStage ? "BOSS STAGE" : "STAGE " + std::to_string(s.stage + 1);
    const std::string info = where + "   LIFE x" + std::to_string(s.lives) + "   ";
    const float infoW = static_cast<float>(g.TextWidth(info, 2)) + 20.f + static_cast<float>(g.TextWidth(std::to_string(s.coins), 2));
    g.Text(info, kCx - infoW * 0.5f, py + 86.f, 2, Hex(0x9a82c0));
    DrawCoinAmount(g, s.coins, kCx - infoW * 0.5f + static_cast<float>(g.TextWidth(info, 2)), py + 86.f, 2, Hex(0xffe27a));
    Divider(g, py + 110.f, pw - 80.f);

    for (int i = 0; i < kPauseCount; ++i) DrawMenuRow(g, kItems[i].label, kCx, py + 128.f + static_cast<float>(i) * 40.f, i == cursor_, frame_);

    Divider(g, py + 290.f, pw - 80.f);
    g.Text(kItems[cursor_].hint, kCx, py + 306.f, 2, Hex(0xe6d6ff), Align::Center);
  }

  void RenderSettings(Graphics& g) const {
    DrawPixelPanel(g, 150.f, 64.f, 660.f, 420.f);
    Title(g, "SETTINGS", 92.f, 5);
    Divider(g, 146.f, 580.f);
    DrawSettingsMenu(g, cursor_, frame_, 174.f, 164.f, 612.f, 400.f);
    DrawKeyHints(g, {{{"UP", "DOWN"}, "SELECT"}, {{"LEFT", "RIGHT"}, "CHANGE"}, {{"ESC"}, "BACK"}}, kCx, 434.f);
  }

  void RenderConfirm(Graphics& g) const {
    const bool restart = screen_ == Screen::ConfirmRestart;
    const float py = 150.f;
    DrawPixelPanel(g, 230.f, py, 500.f, 240.f);
    Title(g, restart ? "RESTART?" : "MAIN MENU?", py + 30.f, 5);
    g.Text(restart ? "COINS AND SCORE FROM THIS STAGE" : "ALL PROGRESS IN THIS RUN", kCx, py + 92.f, 2, Hex(0xe6d6ff), Align::Center);
    g.Text("WILL BE LOST", kCx, py + 114.f, 2, Hex(0xe6d6ff), Align::Center);
    DrawMenuRow(g, "YES", kCx - 100.f, py + 162.f, cursor_ == 0, frame_, Hex(0xff9ad0));
    DrawMenuRow(g, "NO", kCx + 100.f, py + 162.f, cursor_ == 1, frame_);
  }

  Screen screen_ = Screen::Main;
  int cursor_ = kResume;
  int frame_ = 0;
};

}  // namespace

std::unique_ptr<PlayPhase> MakePausedPhase() { return std::make_unique<PausedPhase>(); }

}  // namespace gengar

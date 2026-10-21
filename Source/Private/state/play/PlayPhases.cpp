#include "state/play/PlayPhases.h"

#include "actor/Player.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "core/Session.h"
#include "core/Game.h"
#include "platform/Graphics.h"
#include "platform/Input.h"

namespace gengar {
namespace {

constexpr float kW = static_cast<float>(cfg::kScreenW);
constexpr float kH = static_cast<float>(cfg::kScreenH);

class PlayingPhase : public PlayPhase {
 public:
  const char* Name() const override { return "Playing"; }
  void Update(PlayMode& m) override {
    Game& game = m.GetGame();
    World& world = m.GetWorld();
    if (game.In().Pressed(Action::Pause)) { m.Phase().Change(MakePausedPhase()); return; }

    world.Update();

    if (!world.GetPlayer().IsAlive()) m.Phase().Change(MakePlayerDeadPhase());
    else if (world.BossDefeated()) game.GoVictory();
    else if (world.StageCleared()) m.Phase().Change(MakeStageClearPhase());
  }
};

class PausedPhase : public PlayPhase {
 public:
  const char* Name() const override { return "Paused"; }
  void Update(PlayMode& m) override {
    if (m.GetGame().In().Pressed(Action::Pause)) m.Phase().Change(MakePlayingPhase());
  }
  void Render(PlayMode&, Graphics& g) override {
    g.FillRect(0, 0, kW, kH, Rgb(10, 0, 25, 153));
    g.Text("PAUSE", kW * 0.5f, 230, 8, Hex(0xffffff), Align::Center);
    g.Text("PRESS ESC TO RESUME", kW * 0.5f, 310, 2, Hex(0xc9b0ee), Align::Center);
  }
};

// 사망 연출 후 목숨을 차감하고 재도전/게임오버로 이어진다.
class PlayerDeadPhase : public PlayPhase {
 public:
  const char* Name() const override { return "PlayerDead"; }
  void Enter(PlayMode&) override { DebugLog("[play] player down"); }
  void Update(PlayMode& m) override {
    ++t_;
    m.GetWorld().UpdateEffects();
    if (t_ > 110) m.GetGame().OnPlayerDefeated();
  }
  void Render(PlayMode&, Graphics& g) override {
    if (t_ > 20) g.FillRect(0, 0, kW, kH, Rgb(10, 0, 25, static_cast<int>(std::min(0.6f, static_cast<float>(t_ - 20) / 80.f) * 255)));
  }

 private:
  int t_ = 0;
};

class StageClearPhase : public PlayPhase {
 public:
  const char* Name() const override { return "StageClear"; }
  void Enter(PlayMode& m) override { DebugLog("[play] STAGE CLEAR (score %d, frame %d)", m.GetGame().Sess().score, m.GetWorld().Frame()); }
  void Update(PlayMode& m) override {
    ++t_;
    m.GetWorld().UpdateEffects();
    if (t_ > 170) m.GetGame().GoShop();
  }
  void Render(PlayMode&, Graphics& g) override {
    g.FillRect(0, 0, kW, kH, Rgb(10, 0, 25, 115));
    g.Text("STAGE CLEAR!", kW * 0.5f, 215, 8, Hex(0xffe27a), Align::Center);
  }

 private:
  int t_ = 0;
};

}  // namespace

std::unique_ptr<PlayPhase> MakePlayingPhase() { return std::make_unique<PlayingPhase>(); }
std::unique_ptr<PlayPhase> MakePausedPhase() { return std::make_unique<PausedPhase>(); }
std::unique_ptr<PlayPhase> MakePlayerDeadPhase() { return std::make_unique<PlayerDeadPhase>(); }
std::unique_ptr<PlayPhase> MakeStageClearPhase() { return std::make_unique<StageClearPhase>(); }

}  // namespace gengar

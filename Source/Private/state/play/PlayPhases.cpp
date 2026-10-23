#include "state/play/PlayPhases.h"

#include <algorithm>
#include <cmath>

#include "actor/Player.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "core/Session.h"
#include "core/Game.h"
#include "platform/Graphics.h"
#include "platform/Audio.h"
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
    else if (world.HatCollected()) m.Phase().Change(MakeHatGetPhase());
    else if (world.StageCleared()) m.Phase().Change(MakeStageClearPhase());
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

// 보물 모자를 되찾음: 모자를 쓰고 기뻐하는 모습을 잠깐 보여 준 뒤 엔딩 시네마틱으로.
class HatGetPhase : public PlayPhase {
 public:
  const char* Name() const override { return "HatGet"; }
  void Enter(PlayMode& m) override {
    m.GetWorld().GetPlayer().SetWearingHat(true);
    m.GetWorld().Snd().Play(Sfx::StageClear);
  }
  void Update(PlayMode& m) override {
    ++t_;
    m.GetWorld().UpdateEffects();
    if (t_ > 150) m.GetGame().GoVictory();
  }
  void Render(PlayMode& m, Graphics& g) override {
    const World& w = m.GetWorld();
    const Vec2 pc = w.GetPlayer().Bounds().Center();
    const float k = std::min(1.f, static_cast<float>(t_) / 30.f);
    g.Glow(pc.x - w.Cam().x, pc.y - 20.f, 90.f + 20.f * std::sin(static_cast<float>(t_) * 0.15f), Rgb(255, 226, 122, static_cast<int>(70.f * k)));
    const int size = t_ < 12 ? 4 + t_ / 3 : 7;  // 살짝 튀어나오는 글자
    g.Text("TREASURE HAT GET!", kW * 0.5f, 150, size, Hex(0xffe27a), Align::Center);
    if (t_ > 120) g.FillRect(0, 0, kW, kH, Rgb(0, 0, 0, static_cast<int>(std::min(1.f, static_cast<float>(t_ - 120) / 30.f) * 255)));
  }

 private:
  int t_ = 0;
};

}  // namespace

std::unique_ptr<PlayPhase> MakePlayingPhase() { return std::make_unique<PlayingPhase>(); }
std::unique_ptr<PlayPhase> MakePlayerDeadPhase() { return std::make_unique<PlayerDeadPhase>(); }
std::unique_ptr<PlayPhase> MakeStageClearPhase() { return std::make_unique<StageClearPhase>(); }
std::unique_ptr<PlayPhase> MakeHatGetPhase() { return std::make_unique<HatGetPhase>(); }

}  // namespace gengar

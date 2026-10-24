#include "state/play/PlayPhases.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "actor/Player.h"
#include "actor/projectile/PlayerSkills.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "core/Session.h"
#include "core/Game.h"
#include "data/Cards.h"
#include "platform/Graphics.h"
#include "platform/Audio.h"
#include "platform/Input.h"
#include "ui/CardPicker.h"
#include "ui/Menu.h"

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
    if (game.In().Pressed(Action::Status)) { m.Phase().Change(MakeStatusPhase()); return; }

    world.Update();

    if (!world.GetPlayer().IsAlive()) m.Phase().Change(MakePlayerDeadPhase());
    else if (world.HatCollected()) m.Phase().Change(MakeHatGetPhase());
    else if (world.StageCleared()) m.Phase().Change(MakeStageClearPhase());
    else if (world.PendingPicks() > 0) m.Phase().Change(MakeCardPickPhase());
  }
};

// 레벨업(또는 보스 체력 단계 전환): 게임을 멈추고 카드를 고른다. 여러 레벨이 한꺼번에 오르면 연달아 띄운다.
class CardPickPhase : public PlayPhase {
 public:
  const char* Name() const override { return "CardPick"; }
  void Enter(PlayMode& m) override { Deal(m); }
  void Update(PlayMode& m) override {
    Game& game = m.GetGame();
    World& world = m.GetWorld();
    world.UpdateEffects();
    switch (picker_.Update(game.In(), game.Snd(), Debug().bot)) {
      case CardPicker::Result::None: return;
      case CardPicker::Result::Reroll:
        if (world.RerollsLeft() > 0) {
          world.UseReroll();  // 리롤 카드: 무료
        } else {
          game.Sess().coins -= world.RerollCost();
          DebugLog("[card] paid reroll -%d (coins left %d)", world.RerollCost(), game.Sess().coins);
          world.PayReroll();
        }
        Deal(m);
        return;
      case CardPicker::Result::Picked:
        ApplyCard(game.Sess(), picker_.Choice(), &world.GetPlayer());
        // 그림자 분신은 스테이지 시작 때 만들어지므로, 스테이지 중에 얻으면 여기서 바로 불러낸다
        if (picker_.Choice() == CardId::Familiar) world.Spawn<Familiar>(world.GetPlayer().Bounds().Center(), game.Sess().Card(CardId::Familiar) - 1);
        break;
    }
    world.ConsumePick();
    if (world.PendingPicks() > 0) Deal(m);
    else m.Phase().Change(MakePlayingPhase());
  }
  void Render(PlayMode& m, Graphics& g) override { picker_.Render(g, m.GetGame().Sess()); }

 private:
  void Deal(PlayMode& m) {
    World& world = m.GetWorld();
    const Session& s = m.GetGame().Sess();
    const Player& p = world.GetPlayer();
    OfferContext ctx;
    ctx.stage = world.StageIndex();
    ctx.hpFull = p.Hp() >= p.MaxHp();
    std::vector<CardId> offer = DrawOffer(s, ctx, OfferSize(s));
    if (offer.empty()) { world.ConsumePick(); m.Phase().Change(MakePlayingPhase()); return; }  // 얻을 카드가 하나도 남지 않음
    const bool boss = world.StageIndex() == cfg::kBossStage;
    picker_.Open(std::move(offer), boss ? "PHASE BONUS!" : "LEVEL UP!  LV " + std::to_string(s.prog.level), world.RerollsLeft(), world.RerollCost(), s.coins);
  }

  CardPicker picker_;
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
  int closeAt_ = -1;  // 키를 누른 프레임 (-1 = 아직)
  static constexpr int kArm = 80;    // 기록이 다 나온 뒤부터 키를 받는다
  static constexpr int kClose = 24;  // 키를 누른 뒤 검게 닫히는 시간

 public:
  const char* Name() const override { return "StageClear"; }
  void Enter(PlayMode& m) override { DebugLog("[play] STAGE CLEAR (score %d, frame %d)", m.GetGame().Sess().score, m.GetWorld().Frame()); }
  void Update(PlayMode& m) override {
    ++t_;
    m.GetWorld().UpdateEffects();
    // 기록이 다 나온 뒤 키를 누르면 화면이 검게 닫히고 상점으로 (봇은 기다리지 않는다)
    if (closeAt_ < 0 && t_ >= kArm && (m.GetGame().In().Pressed(Action::Confirm) || Debug().bot)) {
      closeAt_ = t_;
      m.GetGame().Snd().Play(Sfx::Select);
    }
    if (closeAt_ >= 0 && t_ > closeAt_ + kClose) m.GetGame().GoShop();
  }
  // 게임 화면과 확실히 갈리도록: 화면을 어둡게 덮고, 금색 테두리 띠가 가운데에서 펼쳐진 뒤
  // 제목 -> 스테이지 이름 -> 기록이 차례로 나오고, 끝에는 검게 닫히며 상점으로 넘어간다.
  void Render(PlayMode& m, Graphics& g) override {
    const Session& s = m.GetGame().Sess();
    const World& w = m.GetWorld();
    const Color gold = Hex(0xffe27a);
    g.FillRect(0, 0, kW, kH, Rgb(6, 0, 16, static_cast<int>(std::min(1.f, static_cast<float>(t_) / 20.f) * 200.f)));

    constexpr float kTop = 140.f, kBottom = 400.f;
    const float open = std::min(1.f, static_cast<float>(t_) / 14.f);  // 띠가 가운데에서 위아래로 열린다
    const float mid = (kTop + kBottom) * 0.5f, half = (kBottom - kTop) * 0.5f * open;
    g.FillRect(0, mid - half, kW, half * 2.f, Rgb(24, 8, 48, 245));
    g.FillRect(0, mid - half - 6.f, kW, 4, gold);
    g.FillRect(0, mid + half + 2.f, kW, 4, gold);
    if (t_ < 14) return;

    const int pop = t_ < 26 ? 4 + (t_ - 14) / 3 : 8;  // 살짝 튀어나오는 제목
    g.Text("STAGE CLEAR!", kW * 0.5f + 5.f, 175.f + 5.f, pop, Rgb(0, 0, 0, 160), Align::Center);
    g.Text("STAGE CLEAR!", kW * 0.5f, 175.f, pop, gold, Align::Center);
    if (t_ >= 30) g.Text(w.GetLevel().Name(), kW * 0.5f, 252.f, 2, Hex(0xc9b0ee), Align::Center);

    // 기록: 점수 / 코인 / 레벨 / 걸린 시간을 하나씩 띄운다
    const int secs = w.Frame() / 60;
    const std::string time = std::to_string(secs / 60) + ":" + (secs % 60 < 10 ? "0" : "") + std::to_string(secs % 60);
    const std::string labels[4] = {"SCORE", "COINS", "LEVEL", "TIME"};
    const std::string values[4] = {std::to_string(s.score), std::to_string(s.coins), std::to_string(s.prog.level), time};
    for (int i = 0; i < 4; ++i) {
      if (t_ < 44 + i * 8) break;
      const float x = kW * 0.5f + (static_cast<float>(i) - 1.5f) * 190.f;
      g.Text(labels[i], x, 290.f, 2, Hex(0x9a88c0), Align::Center);
      g.Text(values[i], x, 316.f, 4, Hex(0xffffff), Align::Center);
    }
    if (t_ >= kArm && closeAt_ < 0 && (t_ / 20) % 3 != 2) DrawKeyHints(g, {{{"Z"}, "CONTINUE TO SHOP"}}, kW * 0.5f, 362.f);
    if (closeAt_ >= 0) g.FillRect(0, 0, kW, kH, Rgb(0, 0, 0, static_cast<int>(std::min(1.f, static_cast<float>(t_ - closeAt_) / static_cast<float>(kClose)) * 255)));
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
std::unique_ptr<PlayPhase> MakeCardPickPhase() { return std::make_unique<CardPickPhase>(); }
std::unique_ptr<PlayPhase> MakeStageClearPhase() { return std::make_unique<StageClearPhase>(); }
std::unique_ptr<PlayPhase> MakeHatGetPhase() { return std::make_unique<HatGetPhase>(); }

}  // namespace gengar

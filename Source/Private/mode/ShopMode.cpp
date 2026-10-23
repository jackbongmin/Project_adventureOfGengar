#include "mode/ShopMode.h"

#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "actor/PhantomArt.h"
#include "art/Art.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "core/Game.h"
#include "data/Cards.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "platform/Input.h"
#include "ui/Menu.h"
#include "ui/StatsPanel.h"

namespace gengar {
namespace {

// 모서리를 도트 한 칸 깎은 상자: 테두리(edge) 2px + 안쪽(fill)
void Box(Graphics& g, float x, float y, float w, float h, Color fill, Color edge) {
  g.FillRect(x + 2.f, y, w - 4.f, h, edge);
  g.FillRect(x, y + 2.f, w, h - 4.f, edge);
  g.FillRect(x + 2.f, y + 2.f, w - 4.f, h - 4.f, fill);
}

}  // namespace

void ShopMode::StartDraw(Game& game) {
  Session& s = game.Sess();
  OfferContext ctx;
  ctx.stage = s.stage;
  ctx.hpFull = true;  // 상점에서는 하트가 늘 가득 (스테이지마다 다시 채워진다) -> 회복 카드는 나오지 않는다
  ctx.guaranteeRare = true;
  std::vector<CardId> offer = DrawOffer(s, ctx, 3);
  if (offer.empty()) { errorTimer_ = 40; game.Snd().Play(Sfx::Error); return; }
  s.coins -= kCardDrawCost;
  ++draws_;
  DebugLog("[shop] draw #%d (coins left %d)", draws_, s.coins);
  game.Snd().Play(Sfx::Buy);
  picker_.Open(std::move(offer), "CARD DRAW", 0);
  picking_ = true;
}

void ShopMode::Update(Game& game) {
  ++frame_;
  if (errorTimer_ > 0) --errorTimer_;
  if (flashTimer_ > 0) --flashTimer_;
  Session& s = game.Sess();

  if (picking_) {
    if (picker_.Update(game.In(), game.Snd(), Debug().bot) == CardPicker::Result::Picked) {
      ApplyCard(s, picker_.Choice(), nullptr);
      picking_ = false;
      flashTimer_ = 24;
    }
    return;
  }

  if (Debug().bot) {  // 검증용: 1초마다 살 수 있으면 뽑고, 코인이 모자라면 출발한다.
    if (frame_ % 60 != 0) return;
    if (s.coins >= kCardDrawCost) StartDraw(game);
    else { DebugLog("[shop] not enough coins, departing"); game.LeaveShop(); }
    return;
  }

  Input& in = game.In();
  if (in.Pressed(Action::Up) || in.Pressed(Action::Down)) { selected_ = 1 - selected_; game.Snd().Play(Sfx::Select); }
  if (!in.Pressed(Action::Confirm)) return;
  if (selected_ == 1) { game.LeaveShop(); return; }
  if (s.coins >= kCardDrawCost) StartDraw(game);
  else { errorTimer_ = 40; game.Snd().Play(Sfx::Error); }
}

void ShopMode::Render(Graphics& g) {
  constexpr float W = static_cast<float>(cfg::kScreenW), H = static_cast<float>(cfg::kScreenH);
  const Session& s = game_.Sess();
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

  // ---- 왼쪽: 캐릭터 패널 ----
  DrawCharacterPanel(g, s, 28.f, 96.f, 400.f, 392.f, frame_);
  if (flashTimer_ > 0) g.FillRect(40.f, 108.f, 376.f, 368.f, Rgb(255, 230, 140, flashTimer_ * 3));

  // ---- 오른쪽: 카드 뽑기 / 출발 ----
  constexpr float kX = 452.f, kWd = 480.f;
  const bool can = s.coins >= kCardDrawCost;
  {
    const bool sel = selected_ == 0;
    const float shake = errorTimer_ > 0 ? std::sin(static_cast<float>(errorTimer_) * 1.3f) * static_cast<float>(errorTimer_) * 0.15f : 0.f;
    const float x = kX + shake, y = 104.f, h = 168.f;
    Box(g, x, y, kWd, h, sel ? Hex(0x3a1a66) : Hex(0x1c0a34), sel ? Hex(0xc58bff) : Hex(0x2e1452));
    // 부채꼴로 겹친 카드 세 장 (등급 테두리 미리보기)
    const Grade fan[3] = {Grade::Rare, Grade::Legend, Grade::Epic};
    for (int i = 0; i < 3; ++i) {
      const float lift = sel ? std::floor(std::sin(t * 0.1f + static_cast<float>(i)) * 3.f) : 0.f;
      const float cx = x + 40.f + static_cast<float>(i) * 34.f, cy = y + 30.f + (i == 1 ? -6.f : 0.f) + lift;
      Box(g, cx, cy, 60.f, 84.f, Hex(0x2a1048), GradeColor(fan[i]));
      g.Text("?", cx + 30.f, cy + 32.f, 3, GradeColor(fan[i]), Align::Center);
    }
    g.Text("DRAW CARD", x + 196.f, y + 30.f, 3, sel ? Hex(0xffe27a) : Hex(0xffffff));
    g.Text("PICK 1 OF 3 CARDS", x + 196.f, y + 66.f, 2, Hex(0xc9b0ee), Align::Left, false);
    g.Text("1 RARE+ GUARANTEED", x + 196.f, y + 88.f, 2, Hex(0x4fa3ff), Align::Left, false);
    if (can) {
      DrawCoinAmount(g, kCardDrawCost, x + kWd - 20.f, y + 128.f, 3, Hex(0xffe27a), Align::Right);
    } else {
      g.Text("NEED", x + 196.f, y + 132.f, 2, Hex(0xff9ab4));
      DrawCoinAmount(g, kCardDrawCost - s.coins, x + 196.f + static_cast<float>(g.TextWidth("NEED ", 2)), y + 132.f, 2, Hex(0xff6a8a));
      DrawCoinAmount(g, kCardDrawCost, x + kWd - 20.f, y + 128.f, 3, Hex(0xff6a8a), Align::Right);
    }
  }
  {
    const bool sel = selected_ == 1;
    const float y = 286.f, h = 60.f;
    Box(g, kX, y, kWd, h, sel ? Hex(0x3a1a66) : Hex(0x2a1048), sel ? Hex(0xc58bff) : Hex(0x2e1452));
    Box(g, kX + 14.f, y + 14.f, 32.f, 32.f, Hex(0x12061f), Hex(0x12061f));
    const float bob = sel ? std::floor(std::sin(t * 0.15f) * 1.5f) : 0.f;
    g.DrawSprite(art::A().departIcon, kX + 18.f, y + 18.f + bob, 2);
    g.Text("DEPART TO " + next, kX + 62.f, y + 22.f, 2, sel ? Hex(0xffe27a) : Hex(0xffffff));
  }
  // 안내
  DrawPixelPanel(g, kX, 362.f, kWd, 126.f);
  DrawPhantom(g, {kX + 54.f, 424.f + std::floor(std::sin(t * 0.07f) * 3.f)}, 1, t, 1.6f);
  g.Text(draws_ == 0 ? "LEVEL UP CARDS COME FROM XP." : "DRAWS THIS VISIT: " + std::to_string(draws_), kX + 104.f, 392.f, 2, Hex(0xe6d6ff), Align::Left, false);
  g.Text("COINS BUY EXTRA CARD DRAWS.", kX + 104.f, 416.f, 2, Hex(0xb8a2dd), Align::Left, false);
  g.Text("EACH DRAW COSTS 50.", kX + 104.f, 440.f, 2, Hex(0xb8a2dd), Align::Left, false);

  // ---- 아래: 키 안내 ----
  DrawKeyHints(g, {{{"UP", "DOWN"}, "SELECT"}, {{"ENTER"}, selected_ == 1 ? "DEPART" : "DRAW"}}, W * 0.5f, 504.f);

  if (picking_) picker_.Render(g, s);
}

}  // namespace gengar

#include "mode/ShopMode.h"

#include <algorithm>
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
#include "world/Level.h"

namespace gengar {
namespace {

constexpr int kLeaveFrames = 24;  // 출발을 누른 뒤 화면이 검게 닫히는 시간

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
  std::vector<CardId> offer = DrawOffer(s, ctx, OfferSize(s));  // 넓은 선택 카드가 있으면 4장
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

  if (leaveTimer_ >= 0) {  // 출발: 검게 닫힌 뒤 다음 스테이지로
    if (++leaveTimer_ >= kLeaveFrames) game.LeaveShop();
    return;
  }
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
    else { DebugLog("[shop] not enough coins, departing"); leaveTimer_ = 0; }
    return;
  }

  Input& in = game.In();
  if (in.Pressed(Action::Up) || in.Pressed(Action::Down)) { selected_ = 1 - selected_; game.Snd().Play(Sfx::Select); }
  if (!in.Pressed(Action::Confirm)) return;
  if (selected_ == 1) { leaveTimer_ = 0; game.Snd().Play(Sfx::Select); return; }
  if (s.coins >= kCardDrawCost) StartDraw(game);
  else { errorTimer_ = 40; game.Snd().Play(Sfx::Error); }
}

void ShopMode::Render(Graphics& g) {
  constexpr float W = static_cast<float>(cfg::kScreenW), H = static_cast<float>(cfg::kScreenH);
  const Session& s = game_.Sess();
  const float t = static_cast<float>(frame_);

  for (float y = 0; y < H; y += 8.f) g.FillRect(0, y, W, 8.f, Lerp(Hex(0x12062a), Hex(0x2d0f4f), y / H));

  // ---- 머리: 제목, 안내 한 줄, 가진 코인 ----
  const bool boss = s.stage + 1 == cfg::kBossStage;
  if (nextName_.empty()) nextName_ = BuildStage(s.stage + 1).Name();
  g.Text("SOUL SHOP", 40.f + 3.f, 22.f + 3.f, 5, Rgb(0, 0, 0, 140));
  g.Text("SOUL SHOP", 40.f, 22.f, 5, Hex(0xd9b8ff));
  g.Text("SPEND COINS, THEN HEAD OUT", 40.f, 70.f, 2, Hex(0xb8a2dd), Align::Left, false);
  {
    const float w = static_cast<float>(g.TextWidth(std::to_string(s.coins), 4)) + 32.f + 8.f + 36.f;
    Box(g, W - 32.f - w, 20.f, w, 50.f, Hex(0x0e0420), Hex(0x8a5ad0));
    DrawCoinAmount(g, s.coins, W - 50.f, 31.f, 4, Hex(0xffe27a), Align::Right);
  }

  // ---- 왼쪽: 캐릭터 패널 ----
  DrawCharacterPanel(g, s, 28.f, 96.f, 400.f, 392.f, frame_);
  if (flashTimer_ > 0) g.FillRect(40.f, 108.f, 376.f, 368.f, Rgb(255, 230, 140, flashTimer_ * 3));

  // ---- 오른쪽: 카드 뽑기 / 출발. 고른 칸은 금색 테두리가 깜빡이고 왼쪽에 화살표가 붙는다 ----
  constexpr float kX = 452.f, kWd = 480.f;
  const bool can = s.coins >= kCardDrawCost;
  const bool blink = (frame_ / 12) % 2 == 0;
  auto cursor = [&](float y, float h) {  // 칸 왼쪽에서 안쪽을 가리키는 도트 삼각형
    const float nx = kX - 20.f + std::floor(std::sin(t * 0.2f) * 3.f);
    for (int i = 0; i < 5; ++i) {
      g.FillRect(nx + static_cast<float>(i) * 2.f, y + h * 0.5f - 10.f + static_cast<float>(i) * 2.f, 2.f, 20.f - static_cast<float>(i) * 4.f, Hex(0xffe27a));
    }
  };
  {
    const bool sel = selected_ == 0;
    const float shake = errorTimer_ > 0 ? std::sin(static_cast<float>(errorTimer_) * 1.3f) * static_cast<float>(errorTimer_) * 0.15f : 0.f;
    const float x = kX + shake, y = 104.f, h = 168.f;
    Box(g, x, y, kWd, h, sel ? Hex(0x3a1a66) : Hex(0x1c0a34), sel ? (blink ? Hex(0xffe27a) : Hex(0xc58bff)) : Hex(0x3a1a66));
    if (sel) cursor(y, h);
    // 부채꼴로 겹친 카드 세 장 (등급 테두리 미리보기)
    const Grade fan[3] = {Grade::Rare, Grade::Legend, Grade::Epic};
    for (int i = 0; i < 3; ++i) {
      const float lift = sel ? std::floor(std::sin(t * 0.1f + static_cast<float>(i)) * 3.f) : 0.f;
      const float cx = x + 40.f + static_cast<float>(i) * 34.f, cy = y + 30.f + (i == 1 ? -6.f : 0.f) + lift;
      Box(g, cx, cy, 60.f, 84.f, Hex(0x2a1048), GradeColor(fan[i]));
      g.Text("?", cx + 30.f, cy + 32.f, 3, GradeColor(fan[i]), Align::Center);
    }
    g.Text("DRAW CARD", x + 196.f, y + 26.f, 3, sel ? Hex(0xffe27a) : Hex(0xffffff));
    g.Text("PICK 1 OF " + std::to_string(OfferSize(s)) + " CARDS", x + 196.f, y + 64.f, 2, Hex(0xe6d6ff), Align::Left, false);
    g.Text("1 RARE+ GUARANTEED", x + 196.f, y + 88.f, 2, Hex(0x7ab8ff), Align::Left, false);
    g.FillRect(x + 196.f, y + 118.f, kWd - 216.f, 2.f, Hex(0x5a2a9a));
    if (can) {
      g.Text("COST", x + 196.f, y + 134.f, 2, Hex(0xb8a2dd), Align::Left, false);
    } else {
      g.Text("NEED", x + 196.f, y + 134.f, 2, Hex(0xff9ab4), Align::Left, false);
      DrawCoinAmount(g, kCardDrawCost - s.coins, x + 196.f + static_cast<float>(g.TextWidth("NEED ", 2)), y + 134.f, 2, Hex(0xff6a8a));
    }
    DrawCoinAmount(g, kCardDrawCost, x + kWd - 20.f, y + 128.f, 3, can ? Hex(0xffe27a) : Hex(0xff6a8a), Align::Right);
  }
  {
    // 출발: 다음 스테이지로 넘어가는 칸. 고르면 초록빛(보스면 분홍)으로 바뀌고 화살표가 앞으로 흐른다
    const bool sel = selected_ == 1;
    const float y = 286.f, h = 84.f;
    const Color go = boss ? Hex(0xff7aa8) : Hex(0x6fe08a);
    Box(g, kX, y, kWd, h, sel ? Rgb(20, 56, 42) : Hex(0x1c0a34), sel ? (blink ? Hex(0xffe27a) : go) : Hex(0x3a1a66));
    if (sel) cursor(y, h);
    Box(g, kX + 16.f, y + 18.f, 48.f, 48.f, Hex(0x12061f), sel ? go : Hex(0x3a1a66));
    const float bob = sel ? std::floor(std::sin(t * 0.15f) * 1.5f) : 0.f;
    g.DrawSprite(art::A().departIcon, kX + 24.f, y + 26.f + bob, 2);
    g.Text(boss ? "FACE THE BOSS" : "NEXT STAGE", kX + 82.f, y + 16.f, 3, sel ? Hex(0xffe27a) : Hex(0xffffff));
    g.Text(nextName_, kX + 82.f, y + 52.f, 2, sel ? go : Hex(0xb8a2dd), Align::Left, false);
    for (int i = 0; i < 3; ++i) {  // >>> 흐르는 화살표
      const float k = std::fmod(t * 0.05f + static_cast<float>(2 - i) / 3.f, 1.f);
      g.Text(">", kX + kWd - 64.f + static_cast<float>(i) * 14.f, y + 26.f, 3, sel ? WithAlpha(go, 0.3f + 0.7f * k) : Hex(0x3a1a66));
    }
  }
  // 안내
  DrawPixelPanel(g, kX, 384.f, kWd, 104.f);
  DrawPhantom(g, {kX + 54.f, 436.f + std::floor(std::sin(t * 0.07f) * 3.f)}, 1, t, 1.4f);
  g.Text(draws_ == 0 ? "LEVEL UP CARDS COME FROM XP." : "DRAWS THIS VISIT: " + std::to_string(draws_), kX + 104.f, 408.f, 2, Hex(0xffffff), Align::Left, false);
  g.Text("COINS BUY EXTRA CARD DRAWS.", kX + 104.f, 432.f, 2, Hex(0xd9c8f5), Align::Left, false);
  g.Text("EACH DRAW COSTS 50.", kX + 104.f, 456.f, 2, Hex(0xd9c8f5), Align::Left, false);

  // ---- 아래: 키 안내 ----
  DrawKeyHints(g, {{{"UP", "DOWN"}, "SELECT"}, {{"Z"}, selected_ == 1 ? "DEPART" : "DRAW"}}, W * 0.5f, 508.f);

  if (picking_) picker_.Render(g, s);
  if (leaveTimer_ >= 0) g.FillRect(0, 0, W, H, Rgb(0, 0, 0, std::min(255, leaveTimer_ * 255 / kLeaveFrames)));
}

}  // namespace gengar

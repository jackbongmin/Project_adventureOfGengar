#include "mode/ShopMode.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

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
constexpr float kX = 452.f, kWd = 480.f;  // 오른쪽 세 칸

// 모서리를 도트 한 칸 깎은 상자: 테두리(edge) 2px + 안쪽(fill)
void Box(Graphics& g, float x, float y, float w, float h, Color fill, Color edge) {
  g.FillRect(x + 2.f, y, w - 4.f, h, edge);
  g.FillRect(x, y + 2.f, w, h - 4.f, edge);
  g.FillRect(x + 2.f, y + 2.f, w - 4.f, h - 4.f, fill);
}

// 공백 기준으로 maxChars 글자 안에 들어가게 줄을 나눈다
std::vector<std::string> WrapText(const std::string& text, size_t maxChars) {
  std::vector<std::string> lines;
  std::string line;
  size_t i = 0;
  while (i < text.size()) {
    const size_t sp = text.find(' ', i);
    const std::string word = text.substr(i, sp == std::string::npos ? std::string::npos : sp - i);
    if (!line.empty() && line.size() + 1 + word.size() > maxChars) { lines.push_back(line); line.clear(); }
    line += (line.empty() ? "" : " ") + word;
    if (sp == std::string::npos) break;
    i = sp + 1;
  }
  if (!line.empty()) lines.push_back(line);
  return lines;
}

// 위쪽 화살표 도트 (강화 칸 아이콘)
void DrawUpArrow(Graphics& g, float cx, float y, Color c) {
  for (int i = 0; i < 6; ++i) g.FillRect(cx - 2.f - static_cast<float>(i) * 2.f, y + static_cast<float>(i) * 2.f, 4.f + static_cast<float>(i) * 4.f, 2.f, c);
  g.FillRect(cx - 4.f, y + 12.f, 8.f, 14.f, c);
}

}  // namespace

int ShopMode::DrawCost() const { return kCardDrawCost + 25 * game_.Sess().shopDraws; }

int ShopMode::UpgradeCost(const Session& s, CardId id) {
  static constexpr int kBase[4] = {60, 90, 140, 200};
  return kBase[static_cast<int>(GetCard(id).grade)] + 20 * s.Card(id);
}

std::vector<CardId> ShopMode::Upgradable(const Session& s) {
  std::vector<CardId> out;
  for (const CardDef& d : AllCards()) {
    if (d.kind != CardKind::Stat && d.kind != CardKind::SpecialStack) continue;
    if (s.Has(d.id) && s.Card(d.id) < d.maxStack) out.push_back(d.id);
  }
  return out;
}

void ShopMode::StartDraw(Game& game) {
  Session& s = game.Sess();
  OfferContext ctx;
  ctx.stage = s.stage;
  ctx.hpFull = true;  // 상점에서는 하트가 늘 가득 (스테이지마다 다시 채워진다) -> 회복 카드는 나오지 않는다
  ctx.guaranteeRare = true;
  std::vector<CardId> offer = DrawOffer(s, ctx, OfferSize(s));  // 넓은 선택 카드가 있으면 4장
  if (offer.empty()) { errorTimer_ = 40; game.Snd().Play(Sfx::Error); return; }
  s.coins -= DrawCost();
  ++draws_;
  ++s.shopDraws;
  DebugLog("[shop] draw #%d (coins left %d, next cost %d)", draws_, s.coins, DrawCost());
  game.Snd().Play(Sfx::Buy);
  picker_.Open(std::move(offer), "CARD DRAW", 0);
  picking_ = true;
}

// 강화 화면: ↑ ↓ 로 카드를 고르고 Z 로 1단계 올린다. Esc 로 돌아간다
void ShopMode::UpdateUpgrade(Game& game) {
  Session& s = game.Sess();
  Input& in = game.In();
  if (upFlash_ > 0) --upFlash_;
  const std::vector<CardId> list = Upgradable(s);
  if (in.Pressed(Action::Pause)) { upgrading_ = false; game.Snd().Play(Sfx::Select); return; }
  if (list.empty()) {
    if (in.Pressed(Action::Confirm)) { upgrading_ = false; game.Snd().Play(Sfx::Select); }
    return;
  }
  const int n = static_cast<int>(list.size());
  upCursor_ = std::clamp(upCursor_, 0, n - 1);
  if (in.Pressed(Action::Up) || in.Pressed(Action::Left)) { upCursor_ = (upCursor_ + n - 1) % n; game.Snd().Play(Sfx::Select); }
  if (in.Pressed(Action::Down) || in.Pressed(Action::Right)) { upCursor_ = (upCursor_ + 1) % n; game.Snd().Play(Sfx::Select); }
  if (!in.Pressed(Action::Confirm)) return;
  const CardId id = list[static_cast<size_t>(upCursor_)];
  const int cost = UpgradeCost(s, id);
  if (s.coins < cost) { errorTimer_ = 40; game.Snd().Play(Sfx::Error); return; }
  s.coins -= cost;
  ApplyCard(s, id, nullptr);
  DebugLog("[shop] upgrade %s -> Lv.%d (-%d coins)", GetCard(id).name, s.Card(id), cost);
  game.Snd().Play(Sfx::Buy);
  flashTimer_ = 24;
  upFlash_ = 30;
  upFlashId_ = id;
  // 최대 단계가 되어 목록에서 빠지면 커서가 다음 줄을 가리킨다
  const int left = static_cast<int>(Upgradable(s).size());
  if (upCursor_ >= left) upCursor_ = std::max(0, left - 1);
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
  if (upgrading_) { UpdateUpgrade(game); return; }

  if (Debug().bot) {  // 검증용: 1초마다 살 수 있으면 뽑고, 코인이 모자라면 출발한다.
    if (frame_ % 60 != 0) return;
    if (s.coins >= DrawCost()) StartDraw(game);
    else { DebugLog("[shop] not enough coins, departing"); leaveTimer_ = 0; }
    return;
  }

  Input& in = game.In();
  if (in.Pressed(Action::Up)) { selected_ = (selected_ + kItemCount - 1) % kItemCount; game.Snd().Play(Sfx::Select); }
  if (in.Pressed(Action::Down)) { selected_ = (selected_ + 1) % kItemCount; game.Snd().Play(Sfx::Select); }
  if (!in.Pressed(Action::Confirm)) return;
  switch (selected_) {
    case kDraw:
      if (s.coins >= DrawCost()) StartDraw(game);
      else { errorTimer_ = 40; game.Snd().Play(Sfx::Error); }
      break;
    case kUpgrade:
      upgrading_ = true;
      upCursor_ = 0;
      game.Snd().Play(Sfx::Select);
      break;
    case kDepart:
      leaveTimer_ = 0;
      game.Snd().Play(Sfx::Select);
      break;
  }
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

  // ---- 오른쪽: 카드 뽑기 / 카드 강화 / 출발. 고른 칸은 금색 테두리가 깜빡이고 왼쪽에 화살표가 붙는다 ----
  const bool blink = (frame_ / 12) % 2 == 0;
  const float shake = errorTimer_ > 0 ? std::sin(static_cast<float>(errorTimer_) * 1.3f) * static_cast<float>(errorTimer_) * 0.15f : 0.f;
  auto cursor = [&](float y, float h) {  // 칸 왼쪽에서 안쪽을 가리키는 도트 삼각형
    const float nx = kX - 20.f + std::floor(std::sin(t * 0.2f) * 3.f);
    for (int i = 0; i < 5; ++i) {
      g.FillRect(nx + static_cast<float>(i) * 2.f, y + h * 0.5f - 10.f + static_cast<float>(i) * 2.f, 2.f, 20.f - static_cast<float>(i) * 4.f, Hex(0xffe27a));
    }
  };
  // 가격 줄: 모자라면 "NEED n" 을 붉게
  auto price = [&](float x, float y, int cost, bool fromLabel) {
    const bool can = s.coins >= cost;
    if (can) g.Text(fromLabel ? "FROM" : "COST", x, y + 6.f, 2, Hex(0xb8a2dd), Align::Left, false);
    else {
      g.Text("NEED", x, y + 6.f, 2, Hex(0xff9ab4), Align::Left, false);
      DrawCoinAmount(g, cost - s.coins, x + static_cast<float>(g.TextWidth("NEED ", 2)), y + 6.f, 2, Hex(0xff6a8a));
    }
    DrawCoinAmount(g, cost, kX + kWd - 20.f, y, 3, can ? Hex(0xffe27a) : Hex(0xff6a8a), Align::Right);
  };
  {  // 카드 뽑기
    const bool sel = selected_ == kDraw;
    const float x = kX + (sel ? shake : 0.f), y = 104.f, h = 150.f;
    Box(g, x, y, kWd, h, sel ? Hex(0x3a1a66) : Hex(0x1c0a34), sel ? (blink ? Hex(0xffe27a) : Hex(0xc58bff)) : Hex(0x3a1a66));
    if (sel) cursor(y, h);
    const Grade fan[3] = {Grade::Rare, Grade::Legend, Grade::Epic};  // 부채꼴로 겹친 카드 세 장
    for (int i = 0; i < 3; ++i) {
      const float lift = sel ? std::floor(std::sin(t * 0.1f + static_cast<float>(i)) * 3.f) : 0.f;
      const float cx = x + 40.f + static_cast<float>(i) * 34.f, cy = y + 26.f + (i == 1 ? -6.f : 0.f) + lift;
      Box(g, cx, cy, 60.f, 84.f, Hex(0x2a1048), GradeColor(fan[i]));
      g.Text("?", cx + 30.f, cy + 32.f, 3, GradeColor(fan[i]), Align::Center);
    }
    g.Text("DRAW CARD", x + 196.f, y + 20.f, 3, sel ? Hex(0xffe27a) : Hex(0xffffff));
    g.Text("PICK 1 OF " + std::to_string(OfferSize(s)) + " CARDS", x + 196.f, y + 54.f, 2, Hex(0xe6d6ff), Align::Left, false);
    g.Text("1 RARE+, +25 EVERY DRAW", x + 196.f, y + 76.f, 2, Hex(0x7ab8ff), Align::Left, false);
    g.FillRect(x + 196.f, y + 102.f, kWd - 216.f, 2.f, Hex(0x5a2a9a));
    price(x + 196.f, y + 112.f, DrawCost(), false);
  }
  {  // 카드 강화
    const bool sel = selected_ == kUpgrade;
    const std::vector<CardId> list = Upgradable(s);
    const float y = 266.f, h = 104.f;
    Box(g, kX, y, kWd, h, sel ? Rgb(60, 36, 20) : Hex(0x1c0a34), sel ? (blink ? Hex(0xffe27a) : Hex(0xffb347)) : Hex(0x3a1a66));
    if (sel) cursor(y, h);
    Box(g, kX + 16.f, y + 22.f, 56.f, 56.f, Hex(0x12061f), sel ? Hex(0xffb347) : Hex(0x3a1a66));
    DrawUpArrow(g, kX + 44.f, y + 34.f + (sel ? std::floor(std::sin(t * 0.15f) * 1.5f) : 0.f), sel ? Hex(0xffe27a) : Hex(0xffb347));
    g.Text("UPGRADE CARD", kX + 90.f, y + 12.f, 3, sel ? Hex(0xffe27a) : Hex(0xffffff));
    g.Text("+1 LEVEL TO A CARD YOU OWN", kX + 90.f, y + 44.f, 2, Hex(0xe6d6ff), Align::Left, false);
    if (list.empty()) g.Text("NOTHING TO UPGRADE YET", kX + 90.f, y + 76.f, 2, Hex(0x9a82c0), Align::Left, false);
    if (!list.empty()) {
      int cheapest = 1 << 20;
      for (CardId id : list) cheapest = std::min(cheapest, UpgradeCost(s, id));
      g.Text(std::to_string(list.size()) + " READY", kX + 90.f, y + 76.f, 2, Hex(0x6fe08a), Align::Left, false);
      DrawCoinAmount(g, cheapest, kX + kWd - 20.f, y + 70.f, 3, s.coins >= cheapest ? Hex(0xffe27a) : Hex(0xff6a8a), Align::Right);
      g.Text("FROM", kX + kWd - 20.f - static_cast<float>(g.TextWidth(std::to_string(cheapest), 3)) - 44.f, y + 76.f, 2, Hex(0xb8a2dd), Align::Right, false);
    }
  }
  {  // 출발: 고르면 초록빛(보스면 분홍)으로 바뀌고 화살표가 앞으로 흐른다
    const bool sel = selected_ == kDepart;
    const float y = 382.f, h = 106.f;
    const Color go = boss ? Hex(0xff7aa8) : Hex(0x6fe08a);
    Box(g, kX, y, kWd, h, sel ? Rgb(20, 56, 42) : Hex(0x1c0a34), sel ? (blink ? Hex(0xffe27a) : go) : Hex(0x3a1a66));
    if (sel) cursor(y, h);
    Box(g, kX + 16.f, y + 29.f, 48.f, 48.f, Hex(0x12061f), sel ? go : Hex(0x3a1a66));
    const float bob = sel ? std::floor(std::sin(t * 0.15f) * 1.5f) : 0.f;
    g.DrawSprite(art::A().departIcon, kX + 24.f, y + 37.f + bob, 2);
    g.Text(boss ? "FACE THE BOSS" : "NEXT STAGE", kX + 82.f, y + 26.f, 3, sel ? Hex(0xffe27a) : Hex(0xffffff));
    g.Text(nextName_, kX + 82.f, y + 62.f, 2, sel ? go : Hex(0xb8a2dd), Align::Left, false);
    for (int i = 0; i < 3; ++i) {  // >>> 흐르는 화살표
      const float k = std::fmod(t * 0.05f + static_cast<float>(2 - i) / 3.f, 1.f);
      g.Text(">", kX + kWd - 64.f + static_cast<float>(i) * 14.f, y + 37.f, 3, sel ? WithAlpha(go, 0.3f + 0.7f * k) : Hex(0x3a1a66));
    }
  }

  // ---- 아래: 키 안내 ----
  const char* action = selected_ == kDraw ? "DRAW" : selected_ == kUpgrade ? "UPGRADE" : "DEPART";
  if (!upgrading_) DrawKeyHints(g, {{{"UP", "DOWN"}, "SELECT"}, {{"Z"}, action}}, W * 0.5f, 508.f);

  if (upgrading_) RenderUpgrade(g, s);
  if (picking_) picker_.Render(g, s);
  if (leaveTimer_ >= 0) g.FillRect(0, 0, W, H, Rgb(0, 0, 0, std::min(255, leaveTimer_ * 255 / kLeaveFrames)));
}

// 강화 화면 (목록형): 위에 설명 한 줄과 가진 코인, 가운데에 올릴 수 있는 카드 줄(아이콘, 이름, 지금 단계 칸, 가격),
// 아래에 고른 카드의 자세한 설명과 "Z 를 누르면 몇 코인에 올린다" 안내. 코인이 모자라면 가격과 안내가 빨갛다.
void ShopMode::RenderUpgrade(Graphics& g, const Session& s) const {
  constexpr float W = static_cast<float>(cfg::kScreenW), H = static_cast<float>(cfg::kScreenH);
  constexpr float px = 50.f, py = 22.f, pw = W - 100.f, ph = 470.f;
  g.FillRect(0, 0, W, H, Rgb(8, 0, 20, 225));
  Box(g, px, py, pw, ph, Hex(0x1c0a34), Hex(0xffb347));

  // ---- 머리: 제목, 무엇을 하는 화면인지, 가진 코인 ----
  g.Text("UPGRADE A CARD", px + 24.f, py + 16.f, 4, Hex(0xffe27a));
  g.Text("PAY COINS TO RAISE A CARD YOU OWN BY 1 LEVEL.", px + 24.f, py + 56.f, 2, Hex(0xe6d6ff), Align::Left, false);
  g.Text("ONLY CARDS WITH LEVELS ARE LISTED. MAXED CARDS ARE HIDDEN.", px + 24.f, py + 76.f, 2, Hex(0x9a82c0), Align::Left, false);
  g.Text("YOUR COINS", px + pw - 24.f, py + 16.f, 2, Hex(0xb8a2dd), Align::Right, false);
  DrawCoinAmount(g, s.coins, px + pw - 24.f, py + 36.f, 3, Hex(0xffe27a), Align::Right);

  const std::vector<CardId> list = Upgradable(s);
  if (list.empty()) {
    g.Text("NOTHING TO UPGRADE YET", W * 0.5f, py + 190.f, 3, Hex(0xc9b0ee), Align::Center);
    g.Text("CARDS LIKE ATTACK UP OR MULTI SHOT CAN GO UP SEVERAL LEVELS.", W * 0.5f, py + 236.f, 2, Hex(0x9a82c0), Align::Center, false);
    g.Text("GET ONE FIRST, THEN COME BACK HERE.", W * 0.5f, py + 258.f, 2, Hex(0x9a82c0), Align::Center, false);
    DrawKeyHints(g, {{{"Z"}, "BACK"}}, W * 0.5f, 508.f);
    return;
  }

  // ---- 목록: 한 번에 5줄, 고른 줄이 보이도록 굴린다 ----
  const int n = static_cast<int>(list.size());
  const int cur = std::clamp(upCursor_, 0, n - 1);
  constexpr int kRows = 5;
  constexpr float kRowH = 48.f, listY = py + 102.f;
  const int top = std::clamp(cur - kRows / 2, 0, std::max(0, n - kRows));
  for (int r = 0; r < kRows && top + r < n; ++r) {
    const int i = top + r;
    const CardId id = list[static_cast<size_t>(i)];
    const CardDef& d = GetCard(id);
    const bool sel = i == cur;
    const float ry = listY + static_cast<float>(r) * kRowH;
    const int cost = UpgradeCost(s, id);
    const bool can = s.coins >= cost;
    Color fill = sel ? Hex(0x3a1a66) : Hex(0x24103f);
    if (upFlash_ > 0 && id == upFlashId_ && (upFlash_ / 4) % 2) fill = Hex(0x6a4a20);
    Box(g, px + 16.f, ry, pw - 32.f, kRowH - 6.f, fill, sel ? ((frame_ / 12) % 2 ? Hex(0xffe27a) : Hex(0xffb347)) : Hex(0x3a1a66));
    if (sel) {  // 왼쪽 화살표
      for (int k = 0; k < 4; ++k) g.FillRect(px + 4.f + static_cast<float>(k) * 2.f, ry + 11.f + static_cast<float>(k) * 2.f, 2.f, 18.f - static_cast<float>(k) * 4.f, Hex(0xffe27a));
    }
    DrawCardIconSlot(g, id, px + 24.f, ry + 3.f, 34.f, 0);
    g.Text(d.name, px + 70.f, ry + 5.f, 2, GradeColor(d.grade));
    g.Text(GradeName(d.grade), px + 70.f, ry + 22.f, 2, WithAlpha(GradeColor(d.grade), 0.75f), Align::Left, false);
    // 단계 칸: 가진 단계는 보라, 이번에 오를 칸은 금색으로 깜빡인다
    const float pip = 12.f, gap = 4.f, pipX = px + 330.f;
    for (int k = 0; k < d.maxStack; ++k) {
      const bool on = k < s.Card(id), next = k == s.Card(id);
      const float x = pipX + static_cast<float>(k) * (pip + gap);
      g.FillRect(x, ry + 6.f, pip, pip, on ? Hex(0xc58bff) : next ? Hex(0xffe27a) : Hex(0x3a1a66));
      if (next && (frame_ / 10) % 2 == 0) g.FillRect(x + 2.f, ry + 8.f, pip - 4.f, pip - 4.f, Hex(0xffffff));
    }
    g.Text("LV " + std::to_string(s.Card(id)) + " > " + std::to_string(s.Card(id) + 1), pipX, ry + 22.f, 2, Hex(0x6fe08a), Align::Left, false);
    DrawCoinAmount(g, cost, px + pw - 30.f, ry + 9.f, 2, can ? Hex(0xffe27a) : Hex(0xff6a8a), Align::Right);
  }
  // 위아래로 더 있으면 오른쪽 여백에 ^ / v 가 깜빡인다
  if ((frame_ / 15) % 2 == 0) {
    if (top > 0) g.Text("^", px + pw - 9.f, listY + 10.f, 2, Hex(0xffe27a), Align::Center, false);
    if (top + kRows < n) g.Text("v", px + pw - 9.f, listY + kRowH * (kRows - 1) + 14.f, 2, Hex(0xffe27a), Align::Center, false);
  }

  // ---- 아래: 고른 카드 자세히 + 할 일 ----
  const CardId id = list[static_cast<size_t>(cur)];
  const int cost = UpgradeCost(s, id);
  const bool can = s.coins >= cost;
  const float dy = listY + kRowH * kRows + 12.f;
  g.FillRect(px + 16.f, dy - 6.f, pw - 32.f, 2.f, Hex(0x5a2a9a));
  float ly = dy + 4.f;
  for (const std::string& line : WrapText(CardDetail(id), 66)) {
    g.Text(line, px + 24.f, ly, 2, Hex(0xe6d6ff), Align::Left, false);
    ly += 18.f;
  }
  const float shake = errorTimer_ > 0 ? std::sin(static_cast<float>(errorTimer_) * 1.3f) * static_cast<float>(errorTimer_) * 0.15f : 0.f;
  const float ay = py + ph - 34.f;
  if (can) {
    g.Text("PRESS Z TO UPGRADE FOR", px + 24.f, ay, 2, Hex(0xffe27a), Align::Left, false);
    DrawCoinAmount(g, cost, px + 24.f + static_cast<float>(g.TextWidth("PRESS Z TO UPGRADE FOR ", 2)), ay, 2, Hex(0xffe27a));
  } else {
    g.Text("NOT ENOUGH COINS: NEED " + std::to_string(cost - s.coins) + " MORE", px + 24.f + shake, ay, 2, Hex(0xff6a8a), Align::Left, false);
  }
  DrawKeyHints(g, {{{"UP", "DOWN"}, "SELECT"}, {{"Z"}, "UPGRADE"}, {{"ESC"}, "BACK"}}, W * 0.5f, 508.f);
}

}  // namespace gengar

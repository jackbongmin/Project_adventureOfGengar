#include "ui/CardPicker.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "art/Art.h"
#include "core/Config.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "platform/Input.h"
#include "ui/Menu.h"

namespace gengar {
namespace {

constexpr float kW = static_cast<float>(cfg::kScreenW);

// 모서리를 도트 한 칸 깎은 상자
void Box(Graphics& g, float x, float y, float w, float h, float p, Color c) {
  g.FillRect(x + p, y, w - 2.f * p, h, c);
  g.FillRect(x, y + p, w, h - 2.f * p, c);
}

// 공백 기준으로 maxChars 글자 안에 들어가게 줄을 나눈다
std::vector<std::string> Wrap(const std::string& text, size_t maxChars) {
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

// 테두리 둘레 위의 점 (0~1) -> 좌표. 영웅 카드의 도는 빛에 쓴다.
Vec2 PerimeterPoint(float x, float y, float w, float h, float k) {
  const float per = 2.f * (w + h);
  float d = std::fmod(k, 1.f) * per;
  if (d < w) return {x + d, y};
  d -= w;
  if (d < h) return {x + w, y + d};
  d -= h;
  if (d < w) return {x + w - d, y + h};
  d -= w;
  return {x, y + h - d};
}

}  // namespace

void DrawCardIconSlot(Graphics& g, CardId id, float x, float y, float size, int stack) {
  const CardDef& d = GetCard(id);
  Box(g, x, y, size, size, 2.f, GradeColor(d.grade));
  Box(g, x + 2.f, y + 2.f, size - 4.f, size - 4.f, 2.f, Hex(0x12061f));
  const int scale = std::max(1, static_cast<int>((size - 4.f) / 12.f));
  const float iconPx = 12.f * static_cast<float>(scale);
  g.DrawSprite(art::A().cardIcon[static_cast<int>(id)], x + (size - iconPx) * 0.5f, y + (size - iconPx) * 0.5f, scale);
  if (stack > 1) g.Text(std::to_string(stack), x + size - 2.f, y + size - 12.f, 2, Hex(0xffffff), Align::Right);
}

void DrawCard(Graphics& g, const CardDef& def, const Session& s, float x, float y, float w, float h, bool selected, int t) {
  const bool legend = def.grade == Grade::Legend;
  const float tf = static_cast<float>(t);
  const float pulse = 0.5f + 0.5f * std::sin(tf * 0.12f);
  // 전설: 테두리 색이 금색과 흰색 사이를 오가며 숨 쉬듯 빛난다
  const Color gc = legend ? Lerp(GradeColor(def.grade), Hex(0xfff6d0), 0.45f * pulse) : GradeColor(def.grade);
  const float border = def.grade == Grade::Epic || legend ? 6.f : 4.f;

  if (legend) {  // 전설: 테두리 바깥으로 번지는 금빛 후광 (안쪽일수록 진하다)
    g.Glow(x + w * 0.5f, y + h * 0.5f, w * 0.8f, Rgb(255, 201, 60, selected ? 60 : 35));
    for (int k = 5; k >= 1; --k) {
      const float spread = static_cast<float>(k) * 4.f;
      const int a = static_cast<int>((70.f - static_cast<float>(k) * 11.f) * (0.55f + 0.45f * pulse));
      Box(g, x - spread, y - spread, w + 2.f * spread, h + 2.f * spread, 4.f, Rgb(255, 214, 90, a));
    }
  }
  Box(g, x + 6.f, y + 8.f, w, h, 4.f, Rgb(0, 0, 0, 140));       // 그림자
  if (selected) Box(g, x - 4.f, y - 4.f, w + 8.f, h + 8.f, 4.f, Hex(0xffffff));
  Box(g, x, y, w, h, 4.f, gc);                                    // 등급 테두리
  Box(g, x + border, y + border, w - 2.f * border, h - 2.f * border, 2.f, Hex(0x1a0b2e));
  // 테두리 안쪽 하이라이트 줄
  g.FillRect(x + border, y + border, w - 2.f * border, 2.f, Lerp(gc, Hex(0xffffff), 0.5f));

  if (def.grade == Grade::Rare) {  // 희귀: 네 모서리 도트 장식
    const Color c = Lerp(gc, Hex(0xffffff), 0.6f);
    for (const Vec2 p : {Vec2{x + 8.f, y + 8.f}, Vec2{x + w - 14.f, y + 8.f}, Vec2{x + 8.f, y + h - 14.f}, Vec2{x + w - 14.f, y + h - 14.f}}) {
      g.FillRect(p.x, p.y, 6.f, 2.f, c);
      g.FillRect(p.x + 2.f, p.y - 2.f, 2.f, 6.f, c);
    }
  }
  if (def.grade == Grade::Epic) {  // 영웅: 테두리를 따라 천천히 도는 작은 빛 한 점 (전설보다 눈에 덜 띄게)
    const Vec2 p = PerimeterPoint(x + 3.f, y + 3.f, w - 6.f, h - 6.f, tf * 0.005f);
    g.FillRect(std::floor(p.x) - 2.f, std::floor(p.y) - 2.f, 4.f, 4.f, Hex(0xf6d0ff));
  }
  if (legend) {  // 전설: 테두리 위에서 반짝이는 별
    for (int k = 0; k < 6; ++k) {
      const float phase = std::sin(tf * 0.12f + static_cast<float>(k) * 1.7f);
      if (phase < 0.3f) continue;
      const Vec2 p = PerimeterPoint(x + 3.f, y + 3.f, w - 6.f, h - 6.f, static_cast<float>(k) / 6.f + 0.04f);
      const float r = std::floor(phase * 3.f) * 2.f;
      g.FillRect(p.x - 1.f, p.y - r - 1.f, 2.f, r * 2.f + 2.f, Hex(0xfff3b0));
      g.FillRect(p.x - r - 1.f, p.y - 1.f, r * 2.f + 2.f, 2.f, Hex(0xfff3b0));
    }
  }

  const float cx = x + w * 0.5f;
  g.Text(GradeName(def.grade), cx, y + 14.f, 2, gc, Align::Center);

  // 아이콘
  constexpr float kIcon = 72.f;
  Box(g, cx - kIcon * 0.5f, y + 38.f, kIcon, kIcon, 4.f, Hex(0x0c0418));
  const float bob = selected ? std::floor(std::sin(tf * 0.15f) * 2.f) : 0.f;
  g.DrawSprite(art::A().cardIcon[static_cast<int>(def.id)], cx - 30.f, y + 44.f + bob, 5);

  // 이름 (길면 두 줄)
  const size_t nameChars = static_cast<size_t>((w - 16.f) / 12.f);
  const std::vector<std::string> name = Wrap(def.name, nameChars);
  float ty = y + 122.f;
  for (const std::string& line : name) { g.Text(line, cx, ty, 2, Hex(0xffffff), Align::Center); ty += 18.f; }

  // 분류 이름표
  const char* cat = CategoryName(def.category);
  const float tagW = static_cast<float>(g.TextWidth(cat, 1)) * 2.f + 12.f;
  Box(g, cx - tagW * 0.5f, ty + 2.f, tagW, 18.f, 2.f, Hex(0x0c0418));
  g.Text(cat, cx, ty + 4.f, 2, CategoryColor(def.category), Align::Center, false);
  ty += 30.f;

  // 효과 설명
  for (const std::string& line : Wrap(def.desc, static_cast<size_t>((w - 20.f) / 12.f))) {
    g.Text(line, cx, ty, 2, Hex(0xc9b0ee), Align::Center, false);
    ty += 18.f;
  }

  // 아래: 단계 칸 / NEW / 필살기 교체
  const float by = y + h - 30.f;
  const int have = s.Card(def.id);
  if (def.kind == CardKind::Consumable) {
    g.Text("NOW", cx, by, 2, Hex(0x6fe08a), Align::Center);
  } else if (def.id == CardId::UltRain || def.id == CardId::UltBlackHole || def.id == CardId::UltNightmare) {
    g.Text("NEW ULT", cx, by, 2, Hex(0xffe27a), Align::Center);
  } else if (def.maxStack == 1) {
    if ((t / 15) % 2 == 0 || !selected) g.Text("NEW!", cx, by, 2, Hex(0xffe27a), Align::Center);
  } else {
    const float pip = 14.f, gap = 4.f;
    const float total = static_cast<float>(def.maxStack) * (pip + gap) - gap;
    for (int k = 0; k < def.maxStack; ++k) {
      const float px = cx - total * 0.5f + static_cast<float>(k) * (pip + gap);
      const bool on = k < have, next = k == have;
      Box(g, px, by, pip, pip, 2.f, on ? Hex(0xc58bff) : next ? Hex(0xffe27a) : Hex(0x3a1a66));
      if (!on && !next) Box(g, px + 2.f, by + 2.f, pip - 4.f, pip - 4.f, 2.f, Hex(0x1a0b2e));
      if (next && (t / 10) % 2 == 0) Box(g, px + 2.f, by + 2.f, pip - 4.f, pip - 4.f, 2.f, Hex(0xffffff));
    }
  }
}

void CardPicker::Open(std::vector<CardId> offer, std::string title, int freeRerolls, int rerollCost, int coins) {
  offer_ = std::move(offer);
  title_ = std::move(title);
  freeRerolls_ = freeRerolls;
  rerollCost_ = rerollCost;
  coins_ = coins;
  cursor_ = 0;
  t_ = 0;
}

CardPicker::Result CardPicker::Update(Input& in, Audio& audio, bool bot) {
  ++t_;
  if (offer_.empty()) return Result::None;
  const int n = static_cast<int>(offer_.size());
  if (t_ == 1) {
    const bool legend = std::any_of(offer_.begin(), offer_.end(), [](CardId id) { return GetCard(id).grade == Grade::Legend; });
    audio.Tone(legend ? 392.f : 330.f, legend ? 1568.f : 990.f, legend ? 0.35f : 0.2f, Wave::Triangle, 0.06f);
  }
  if (t_ < kArmFrames) return Result::None;
  if (bot) {  // 자동 플레이: 등급이 높은 것, 같으면 공격 카드
    if (t_ < kArmFrames + 15) return Result::None;
    int best = 0;
    auto score = [&](int i) {
      const CardDef& d = GetCard(offer_[static_cast<size_t>(i)]);
      return static_cast<int>(d.grade) * 10 + (d.category == CardCategory::Attack ? 1 : 0);
    };
    for (int i = 1; i < n; ++i) if (score(i) > score(best)) best = i;
    cursor_ = best;
    audio.Play(Sfx::Buy);
    return Result::Picked;
  }
  if (in.Pressed(Action::Left)) { cursor_ = (cursor_ + n - 1) % n; audio.Play(Sfx::Select); }
  if (in.Pressed(Action::Right)) { cursor_ = (cursor_ + 1) % n; audio.Play(Sfx::Select); }
  if (in.Pressed(Action::Reroll)) {
    if (CanReroll()) { audio.Play(freeRerolls_ > 0 ? Sfx::Select : Sfx::Buy); return Result::Reroll; }
    if (rerollCost_ >= 0) audio.Tone(180.f, 120.f, 0.12f, Wave::Square, 0.05f);  // 코인이 모자람
  }
  if (in.Pressed(Action::Confirm)) { audio.Play(Sfx::Buy); return Result::Picked; }
  return Result::None;
}

void CardPicker::Render(Graphics& g, const Session& s) const {
  constexpr float H = static_cast<float>(cfg::kScreenH);
  g.FillRect(0, 0, kW, H, Rgb(8, 0, 20, 185));
  const float bob = std::floor(std::sin(static_cast<float>(t_) * 0.08f) * 3.f);
  g.Text(title_, kW * 0.5f + 4.f, 44.f + bob + 4.f, 5, Rgb(0, 0, 0, 140), Align::Center);
  g.Text(title_, kW * 0.5f, 44.f + bob, 5, Hex(0x7ae0ff), Align::Center);
  g.Text("CHOOSE 1 CARD", kW * 0.5f, 92.f, 2, Hex(0xc9b0ee), Align::Center);

  const int n = static_cast<int>(offer_.size());
  const float w = n >= 4 ? 196.f : 220.f, h = 300.f, gap = n >= 4 ? 18.f : 30.f;
  const float total = static_cast<float>(n) * w + static_cast<float>(n - 1) * gap;
  for (int i = 0; i < n; ++i) {
    const int since = t_ - i * 6;  // 왼쪽부터 차례로 펼쳐진다
    if (since <= 0) continue;
    const float k = std::min(1.f, static_cast<float>(since) / 12.f);
    const CardDef& def = GetCard(offer_[static_cast<size_t>(i)]);
    const bool sel = i == cursor_ && t_ >= kArmFrames;
    const float x = (kW - total) * 0.5f + static_cast<float>(i) * (w + gap);
    const float y = 120.f - (sel ? 14.f : 0.f) + (1.f - k) * 40.f;
    if (k < 1.f) {  // 펼쳐지는 중: 뒷면이 가로로 커진다
      const float cw = std::max(4.f, w * k);
      Box(g, x + (w - cw) * 0.5f, y, cw, h, 4.f, GradeColor(def.grade));
      Box(g, x + (w - cw) * 0.5f + 4.f, y + 4.f, std::max(0.f, cw - 8.f), h - 8.f, 2.f, Hex(0x2a1048));
      continue;
    }
    DrawCard(g, def, s, x, y, w, h, sel, since);
    if (def.grade == Grade::Legend && since < 24) {  // 전설: 열리는 순간 하얗게 번쩍
      Box(g, x, y, w, h, 4.f, Rgb(255, 250, 220, static_cast<int>((1.f - static_cast<float>(since - 12) / 12.f) * 220.f)));
    }
  }

  if (t_ < kArmFrames) return;
  if (freeRerolls_ > 0 || rerollCost_ >= 0) {
    // 무료(리롤 카드)가 남아 있으면 먼저 쓰고, 없으면 코인을 낸다. 코인이 모자라면 필요한 만큼 붉게 보여 준다.
    static std::string reroll;
    if (freeRerolls_ > 0) reroll = "FREE REROLL (" + std::to_string(freeRerolls_) + ")";
    else if (coins_ >= rerollCost_) reroll = "REROLL  -" + std::to_string(rerollCost_) + " COINS";
    else reroll = "REROLL  NEED " + std::to_string(rerollCost_) + " COINS";
    DrawKeyHints(g, {{{"LEFT", "RIGHT"}, "SELECT"}, {{"Z"}, "PICK"}, {{"R"}, reroll.c_str()}}, kW * 0.5f, 470.f);
    DrawCoinAmount(g, coins_, kW - 24.f, 462.f, 3, CanReroll() ? Hex(0xffe27a) : Hex(0xff6a8a), Align::Right);
  } else {
    DrawKeyHints(g, {{{"LEFT", "RIGHT"}, "SELECT"}, {{"Z"}, "PICK"}}, kW * 0.5f, 470.f);
  }
}

}  // namespace gengar

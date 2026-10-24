// 능력치·카드 창 (Tab): 게임을 멈추고 왼쪽에 캐릭터 패널, 오른쪽에 얻은 카드 격자를 보여 준다.
// 카드에 마우스를 올리거나 방향키로 고르면 아래 칸에 자세한 설명이 나온다. Tab 이나 Esc 로 닫는다.
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "core/Config.h"
#include "core/Game.h"
#include "core/Session.h"
#include "data/Cards.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "platform/Input.h"
#include "state/play/PlayPhases.h"
#include "ui/CardPicker.h"
#include "ui/Menu.h"
#include "ui/StatsPanel.h"

namespace gengar {
namespace {

constexpr float kW = static_cast<float>(cfg::kScreenW);
constexpr float kH = static_cast<float>(cfg::kScreenH);

// 오른쪽 카드 칸 배치
constexpr float kPanelX = 440.f, kPanelY = 64.f, kPanelW = 496.f, kPanelH = 440.f;
constexpr float kGridX = kPanelX + 20.f, kGridY = kPanelY + 52.f;
constexpr float kDetailY = kPanelY + kPanelH - 168.f;  // 설명 칸 윗변 (설명 4줄까지)
constexpr float kGap = 8.f;

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

class StatusPhase : public PlayPhase {
 public:
  const char* Name() const override { return "Status"; }

  void Enter(PlayMode& m) override {
    m.GetGame().Snd().Play(Sfx::Select);
    lastMouse_ = m.GetGame().In().Mouse();
  }

  void Update(PlayMode& m) override {
    ++frame_;
    Game& game = m.GetGame();
    Input& in = game.In();
    if (in.Pressed(Action::Status) || in.Pressed(Action::Pause)) {
      game.Snd().Play(Sfx::Select);
      m.Phase().Change(MakePlayingPhase());
      return;
    }
    Collect(game.Sess());
    const int n = static_cast<int>(cards_.size());
    if (n == 0) return;
    const int per = PerRow();
    // 방향키: 격자 안에서 이동
    if (in.Pressed(Action::Left)) Move(game, sel_ - 1);
    if (in.Pressed(Action::Right)) Move(game, sel_ + 1);
    if (in.Pressed(Action::Up)) Move(game, sel_ - per);
    if (in.Pressed(Action::Down)) Move(game, sel_ + per);
    // 마우스: 움직였을 때만 올려 둔 칸으로 고른다 (가만히 있으면 방향키 선택을 덮어쓰지 않는다)
    const Vec2 mouse = in.Mouse();
    if (in.MouseSeen() && (mouse.x != lastMouse_.x || mouse.y != lastMouse_.y)) {
      lastMouse_ = mouse;
      const int hit = SlotAt(mouse);
      if (hit >= 0 && hit != sel_) { sel_ = hit; game.Snd().Play(Sfx::Select); }
    }
    sel_ = std::clamp(sel_, 0, n - 1);
  }

  void Render(PlayMode& m, Graphics& g) override {
    const Session& s = m.GetGame().Sess();
    Collect(s);
    g.FillRect(0, 0, kW, kH, Rgb(8, 0, 20, 250));
    g.Text("STATUS", 28.f + 3.f, 16.f + 3.f, 4, Rgb(0, 0, 0, 140));
    g.Text("STATUS", 28.f, 16.f, 4, Hex(0xd9b8ff));
    g.Text("STAGE " + std::to_string(s.stage + 1), kW - 28.f, 26.f, 2, Hex(0xb8a2dd), Align::Right, false);

    DrawCharacterPanel(g, s, 24.f, kPanelY, 400.f, kPanelH, frame_);

    DrawPixelPanel(g, kPanelX, kPanelY, kPanelW, kPanelH);
    g.Text("CARDS", kGridX, kPanelY + 20.f, 2, Hex(0xd9b8ff));
    g.Text(std::to_string(cards_.size()), kGridX + 72.f, kPanelY + 20.f, 2, Hex(0x9a82c0), Align::Left, false);
    g.Text("MOUSE OVER A CARD FOR DETAILS", kPanelX + kPanelW - 20.f, kPanelY + 20.f, 2, Hex(0x7a62a0), Align::Right, false);

    const int n = static_cast<int>(cards_.size());
    if (n == 0) {
      g.Text("NO CARDS YET", kPanelX + kPanelW * 0.5f, kGridY + 80.f, 3, Hex(0x6a4a9a), Align::Center);
    }
    const float slot = SlotSize();
    for (int i = 0; i < n; ++i) {
      const Rect r = SlotRect(i);
      if (i == sel_) {  // 고른 칸: 흰 테두리가 깜빡인다
        const Color edge = (frame_ / 10) % 2 ? Hex(0xffffff) : Hex(0xffe27a);
        g.FillRect(r.x - 4.f, r.y - 4.f, r.w + 8.f, r.h + 8.f, edge);
      }
      DrawCardIconSlot(g, cards_[static_cast<size_t>(i)], r.x, r.y, slot, s.Card(cards_[static_cast<size_t>(i)]));
    }

    // ---- 설명 칸 ----
    g.FillRect(kPanelX + 14.f, kDetailY, kPanelW - 28.f, 2.f, Hex(0x5a2a9a));
    if (n > 0) {
      const CardId id = cards_[static_cast<size_t>(sel_)];
      const CardDef& d = GetCard(id);
      const float x = kGridX, y = kDetailY + 14.f;
      g.Text(d.name, x, y, 3, GradeColor(d.grade));
      const std::string lv = d.maxStack >= 99 ? "" : "LV " + std::to_string(s.Card(id)) + "/" + std::to_string(d.maxStack);
      g.Text(lv, kPanelX + kPanelW - 20.f, y + 4.f, 2, Hex(0xffe27a), Align::Right, false);
      const float gx = x + static_cast<float>(g.TextWidth(GradeName(d.grade), 2)) + 12.f;
      g.Text(GradeName(d.grade), x, y + 34.f, 2, GradeColor(d.grade), Align::Left, false);
      g.Text(CategoryName(d.category), gx, y + 34.f, 2, CategoryColor(d.category), Align::Left, false);
      float ly = y + 58.f;
      for (const std::string& line : WrapText(CardDetail(id), 37)) {
        g.Text(line, x, ly, 2, Hex(0xe6d6ff), Align::Left, false);
        ly += 19.f;
      }
    }

    DrawKeyHints(g, {{{"LEFT", "RIGHT"}, "SELECT"}, {{"TAB"}, "CLOSE"}}, kW * 0.5f, 516.f);
  }

 private:
  // 얻은 카드 (능력치 카드 포함, 회복처럼 바로 쓰고 끝나는 카드는 뺀다). 표 순서 = 분류 순서
  void Collect(const Session& s) {
    cards_.clear();
    for (const CardDef& d : AllCards()) {
      if (d.kind != CardKind::Consumable && s.Has(d.id)) cards_.push_back(d.id);
    }
    if (sel_ >= static_cast<int>(cards_.size())) sel_ = std::max(0, static_cast<int>(cards_.size()) - 1);
  }
  // 카드가 많으면 칸을 줄여 설명 칸 위에 다 들어가게 한다
  float SlotSize() const {
    const float areaH = kDetailY - 12.f - kGridY;
    for (float size : {48.f, 40.f, 32.f}) {
      const int per = static_cast<int>((kPanelW - 40.f + kGap) / (size + kGap));
      const int rows = (static_cast<int>(cards_.size()) + per - 1) / per;
      if (static_cast<float>(rows) * (size + kGap) - kGap <= areaH) return size;
    }
    return 32.f;
  }
  int PerRow() const { return static_cast<int>((kPanelW - 40.f + kGap) / (SlotSize() + kGap)); }
  Rect SlotRect(int i) const {
    const float size = SlotSize();
    const int per = PerRow();
    return {kGridX + static_cast<float>(i % per) * (size + kGap), kGridY + static_cast<float>(i / per) * (size + kGap), size, size};
  }
  int SlotAt(Vec2 p) const {
    for (int i = 0; i < static_cast<int>(cards_.size()); ++i) {
      const Rect r = SlotRect(i);
      if (p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h) return i;
    }
    return -1;
  }
  void Move(Game& game, int to) {
    const int n = static_cast<int>(cards_.size());
    if (to < 0 || to >= n || to == sel_) return;
    sel_ = to;
    game.Snd().Play(Sfx::Select);
  }

  std::vector<CardId> cards_;
  int sel_ = 0;
  int frame_ = 0;
  Vec2 lastMouse_{};
};

}  // namespace

std::unique_ptr<PlayPhase> MakeStatusPhase() { return std::make_unique<StatusPhase>(); }

}  // namespace gengar

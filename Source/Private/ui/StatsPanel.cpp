#include "ui/StatsPanel.h"

#include <cstdio>
#include <string>

#include "actor/PhantomArt.h"
#include "art/Art.h"
#include "core/Session.h"
#include "data/Cards.h"
#include "data/PlayerStats.h"
#include "platform/Graphics.h"
#include "ui/CardPicker.h"
#include "ui/Menu.h"

namespace gengar {
namespace {

std::string Fmt1(float v) {
  char buf[16];
  std::snprintf(buf, sizeof(buf), "%.1f", static_cast<double>(v));
  return buf;
}

const char* UltName(UltType u) {
  switch (u) {
    case UltType::Pulse: return "PULSE";
    case UltType::Rain: return "RAIN";
    case UltType::BlackHole: return "BLACK HOLE";
    case UltType::Nightmare: return "NIGHTMARE";
  }
  return "";
}

CardId UltIcon(UltType u) {
  switch (u) {
    case UltType::Pulse: return CardId::UltPower;
    case UltType::Rain: return CardId::UltRain;
    case UltType::BlackHole: return CardId::UltBlackHole;
    case UltType::Nightmare: return CardId::UltNightmare;
  }
  return CardId::UltPower;
}

}  // namespace

void DrawCharacterPanel(Graphics& g, const Session& s, float x, float y, float w, float h, int frame) {
  DrawPixelPanel(g, x, y, w, h);
  const float t = static_cast<float>(frame);

  // ---- 머리: 팬텀 + 레벨 + 경험치 바 ----
  DrawPhantom(g, {x + 48.f, y + 46.f + std::floor(std::sin(t * 0.07f) * 2.f)}, 1, t, 1.4f);
  g.Text("LV " + std::to_string(s.prog.level), x + 90.f, y + 24.f, 3, Hex(0x7ae0ff));
  const float bx = x + 90.f, bw = w - 116.f, by = y + 54.f;
  const float ratio = static_cast<float>(s.prog.xp) / static_cast<float>(XpToNext(s.prog.level));
  g.FillRect(bx, by, bw, 10.f, Hex(0x0c0418));
  g.FillRect(bx + 2.f, by + 2.f, std::floor((bw - 4.f) * ratio / 2.f) * 2.f, 6.f, Hex(0x7ae0ff));
  g.Text(std::to_string(s.prog.xp) + "/" + std::to_string(XpToNext(s.prog.level)) + " XP", bx + bw, by + 16.f, 2, Hex(0x9a82c0), Align::Right, false);

  // ---- 능력치: 2열, 아이콘 + 이름 + 값 ----
  struct Row { CardId icon; const char* label; std::string value; };
  const Row rows[] = {
      {CardId::AttackUp, "ATK", std::to_string(static_cast<int>(stats::Damage(s)))},
      {CardId::FireRate, "RATE", Fmt1(60.f / static_cast<float>(stats::FireCooldown(s))) + "/S"},
      {CardId::MultiShot, "SHOT", "X" + std::to_string(stats::ShotCount(s))},
      {CardId::RangeUp, "RANGE", std::to_string(stats::ShotRange(s))},
      {CardId::MaxHeart, "HEART", std::to_string(stats::MaxHp(s))},
      {CardId::ExtraLife, "LIFE", std::to_string(s.lives)},
      {CardId::MoveSpeed, "SPEED", Fmt1(stats::MoveSpeed(s))},
      {CardId::AirJump, "2X JUMP", std::to_string(stats::AirJumps(s))},
      {CardId::DashCharge, "DASH", std::to_string(stats::DashCharges(s))},
      {CardId::GaugeUp, "GAUGE", "X" + Fmt1(stats::GaugeMultiplier(s))},
  };
  // 줄마다 옅은 띠를 깔아 눈이 줄을 따라가게 하고, 카드로 올린 능력치는 초록 값으로 구분한다
  const float colW = (w - 40.f) * 0.5f;
  float ry = y + 92.f;
  for (int i = 0; i < 10; ++i) {
    const Row& r = rows[i];
    const float rx = x + 20.f + static_cast<float>(i % 2) * colW;
    if (i % 2 == 0 && i > 0) ry += 26.f;
    if (i % 4 == 0) g.FillRect(x + 14.f, ry - 3.f, w - 28.f, 24.f, Rgb(255, 255, 255, 14));
    const bool up = s.Card(r.icon) > 0;
    g.DrawSprite(art::A().cardIcon[static_cast<int>(r.icon)], rx, ry, 2);
    g.Text(r.label, rx + 30.f, ry + 5.f, 2, Hex(0xc9b0ee), Align::Left, false);
    g.Text(r.value, rx + colW - 14.f, ry + 5.f, 2, up ? Hex(0x6fe08a) : Hex(0xffffff), Align::Right, false);
  }
  ry += 30.f;

  // 장착 필살기
  g.DrawSprite(art::A().cardIcon[static_cast<int>(UltIcon(s.prog.ult))], x + 20.f, ry, 2);
  g.Text("ULT", x + 50.f, ry + 5.f, 2, Hex(0xc9b0ee), Align::Left, false);
  g.Text(UltName(s.prog.ult), x + w - 34.f, ry + 5.f, 2, Hex(0xffe27a), Align::Right, false);
  ry += 30.f;

  // ---- 얻은 특수 카드: 등급 테두리 아이콘 격자 ----
  for (float dx = x + 20.f; dx < x + w - 20.f; dx += 12.f) g.FillRect(dx, ry, 4.f, 4.f, Hex(0x5a2a9a));
  ry += 12.f;
  g.Text("SKILLS", x + 20.f, ry, 2, Hex(0xd9b8ff));
  ry += 22.f;
  constexpr float kSlot = 36.f, kGap = 6.f;
  const int perRow = static_cast<int>((w - 40.f + kGap) / (kSlot + kGap));
  int n = 0;
  for (const CardDef& d : AllCards()) {
    if (d.kind == CardKind::Stat || d.kind == CardKind::Consumable || !s.Has(d.id)) continue;
    const float sx = x + 20.f + static_cast<float>(n % perRow) * (kSlot + kGap);
    const float sy = ry + static_cast<float>(n / perRow) * (kSlot + kGap);
    if (sy + kSlot > y + h - 16.f) break;
    DrawCardIconSlot(g, d.id, sx, sy, kSlot, s.Card(d.id));
    ++n;
  }
  if (n == 0) g.Text("NONE YET", x + 20.f, ry + 6.f, 2, Hex(0x6a4a9a), Align::Left, false);
}

}  // namespace gengar

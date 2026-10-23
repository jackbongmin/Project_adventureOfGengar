#include "data/Cards.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "actor/Player.h"
#include "core/DebugOptions.h"
#include "core/Random.h"
#include "data/PlayerStats.h"

namespace gengar {
namespace {

using G = Grade;
using C = CardCategory;
using K = CardKind;

// 기획서 카드 목록과 같은 순서 (CardId 순서)
const std::vector<CardDef>& Table() {
  static const std::vector<CardDef> table = {
      {CardId::AttackUp, "ATTACK UP", "SHADOW BALL DAMAGE +1", G::Common, C::Attack, K::Stat, 5},
      {CardId::FireRate, "FIRE RATE", "SHOOT 15% FASTER", G::Common, C::Attack, K::Stat, 4},
      {CardId::RangeUp, "RANGE UP", "BALLS FLY 20% FARTHER", G::Common, C::Attack, K::Stat, 3},
      {CardId::ShotSpeed, "SHOT SPEED", "BALLS FLY 20% FASTER", G::Common, C::Attack, K::Stat, 2},
      {CardId::Knockback, "KNOCKBACK", "HITS PUSH ENEMIES BACK", G::Common, C::Attack, K::Stat, 2},
      {CardId::Critical, "CRITICAL", "+15% CHANCE OF 2X DAMAGE", G::Rare, C::Attack, K::Stat, 3},
      {CardId::Pierce, "PIERCE", "BALLS PASS THROUGH 1 MORE ENEMY", G::Rare, C::Attack, K::SpecialStack, 2},
      {CardId::BackShot, "BACK SHOT", "ALSO SHOOT BEHIND YOU (50% DAMAGE)", G::Rare, C::Attack, K::Special, 1},
      {CardId::Execute, "EXECUTE", "+50% DAMAGE TO FOES UNDER 30% HP", G::Rare, C::Attack, K::Special, 1},
      {CardId::MultiShot, "MULTI SHOT", "+1 BALL PER SHOT", G::Epic, C::Attack, K::SpecialStack, 2},
      {CardId::Bounce, "BOUNCE", "BALLS BOUNCE OFF WALLS 1 MORE TIME", G::Epic, C::Attack, K::SpecialStack, 2},
      {CardId::BurstShot, "BURST SHOT", "HITS EXPLODE FOR 50% DAMAGE AROUND", G::Epic, C::Attack, K::Special, 1},
      {CardId::Curse, "CURSE", "HIT FOES TAKE +25% DAMAGE FOR 3S", G::Epic, C::Attack, K::Special, 1},
      {CardId::Homing, "HOMING", "BALLS CURVE TOWARD THE NEAREST FOE", G::Legend, C::Attack, K::Special, 1},
      {CardId::Split, "SPLIT", "HITS SPLIT INTO 3 SMALL BALLS", G::Legend, C::Attack, K::Special, 1},
      {CardId::ChargeShot, "CHARGE SHOT", "HOLD Z 1S: GIANT PIERCING BALL 3X DAMAGE", G::Legend, C::Attack, K::Special, 1},

      {CardId::MaxHeart, "MAX HEART", "+1 MAX HEART", G::Common, C::Defense, K::Stat, 4},
      {CardId::Heal, "HEAL", "RESTORE 2 HEARTS NOW", G::Common, C::Defense, K::Consumable, 99},
      {CardId::LongGuard, "LONG GUARD", "+50% SAFE TIME AFTER A HIT", G::Rare, C::Defense, K::Stat, 2},
      {CardId::SoulDrain, "SOUL DRAIN", "HEAL 1 HEART EVERY 12 KILLS", G::Rare, C::Defense, K::Special, 1},
      {CardId::Thorns, "THORNS", "FOES THAT BUMP YOU TAKE 3 DAMAGE", G::Rare, C::Defense, K::Special, 1},
      {CardId::Shield, "SHIELD", "BLOCK 1 HIT EACH STAGE", G::Epic, C::Defense, K::Special, 1},
      {CardId::ExtraLife, "EXTRA LIFE", "+1 LIFE", G::Epic, C::Defense, K::SpecialStack, 3},

      {CardId::MoveSpeed, "MOVE SPEED", "RUN FASTER", G::Common, C::Move, K::Stat, 3},
      {CardId::JumpUp, "JUMP UP", "JUMP 8% HIGHER", G::Common, C::Move, K::Stat, 2},
      {CardId::AirJump, "AIR JUMP", "+1 JUMP IN MID-AIR", G::Rare, C::Move, K::SpecialStack, 2},
      {CardId::Float, "FLOAT", "HOLD JUMP IN THE AIR TO FALL SLOWLY", G::Rare, C::Move, K::Special, 1},

      {CardId::DashRange, "DASH RANGE", "DASH 25% FARTHER", G::Common, C::Dash, K::Stat, 2},
      {CardId::DashRecharge, "DASH RECHARGE", "DASH REFILLS 20% FASTER", G::Common, C::Dash, K::Stat, 2},
      {CardId::DashCharge, "DASH CHARGE", "+1 DASH SLOT: DASH AGAIN IN A ROW", G::Rare, C::Dash, K::SpecialStack, 2},
      {CardId::AirDash, "AIR DASH", "+1 DASH IN MID-AIR", G::Rare, C::Dash, K::Special, 1},
      {CardId::PhaseDash, "PHASE DASH", "IGNORE ALL DAMAGE WHILE DASHING", G::Epic, C::Dash, K::Special, 1},
      {CardId::ShadowRush, "SHADOW RUSH", "DASH THROUGH FOES FOR 2X DAMAGE", G::Epic, C::Dash, K::Special, 1},
      {CardId::Afterimage, "AFTERIMAGE", "DASH LEAVES A GHOST THAT EXPLODES 3X", G::Legend, C::Dash, K::Special, 1},

      {CardId::GaugeUp, "GAUGE UP", "ULT GAUGE FILLS 20% FASTER", G::Common, C::Ult, K::Stat, 3},
      {CardId::UltPower, "ULT POWER", "+30% ULT DAMAGE", G::Rare, C::Ult, K::Stat, 3},
      {CardId::HeadStart, "HEAD START", "START EACH STAGE WITH 30% GAUGE", G::Common, C::Ult, K::Special, 1},
      {CardId::UltRain, "SHADOW RAIN", "ULT: SHADOW BALLS RAIN ON FOES FOR 3S", G::Rare, C::Ult, K::Special, 1},
      {CardId::UltBlackHole, "BLACK HOLE", "ULT: A BLACK HOLE PULLS AND GRINDS FOES", G::Epic, C::Ult, K::Special, 1},
      {CardId::UltNightmare, "NIGHTMARE", "ULT: STOP TIME 4S. 2X DAMAGE. NO HARM", G::Legend, C::Ult, K::Special, 1},

      {CardId::Magnet, "MAGNET", "PULL COINS FROM FARTHER", G::Common, C::Utility, K::Stat, 2},
      {CardId::XpBoost, "XP BOOST", "+20% XP", G::Common, C::Utility, K::Stat, 3},
      {CardId::ComboKeep, "COMBO KEEP", "COMBO LASTS 50% LONGER", G::Common, C::Utility, K::Special, 1},
      {CardId::Greed, "GREED", "+25% COINS", G::Rare, C::Utility, K::Stat, 2},
      {CardId::Luck, "LUCK", "BETTER CARDS SHOW UP MORE", G::Rare, C::Utility, K::Stat, 3},
      {CardId::Reroll, "REROLL", "+1 REROLL OF CARDS EACH STAGE", G::Epic, C::Utility, K::SpecialStack, 2},
      {CardId::LastStand, "LAST STAND", "AT 1 HEART: +50% DAMAGE +30% FIRE RATE", G::Epic, C::Utility, K::Special, 1},
      {CardId::MoreChoice, "MORE CHOICE", "SEE 4 CARDS WHEN YOU LEVEL UP", G::Epic, C::Utility, K::Special, 1},
      {CardId::Familiar, "FAMILIAR", "A MINI GHOST FOLLOWS AND SHOOTS FOES", G::Legend, C::Utility, K::Special, 1},
  };
  return table;
}

// 등급 확률 (스테이지 1 / 2 / 3). 행운 카드 1단계마다 일반 -5%p, 희귀 +2, 영웅 +2, 전설 +1.
constexpr float kGradeOdds[3][4] = {
    {0.44f, 0.32f, 0.18f, 0.06f},
    {0.36f, 0.34f, 0.22f, 0.08f},
    {0.28f, 0.36f, 0.26f, 0.10f},
};
constexpr float kLuckShift[4] = {-0.05f, 0.02f, 0.02f, 0.01f};

int UltRank(UltType u) { return static_cast<int>(u); }
UltType UltOf(CardId id) {
  switch (id) {
    case CardId::UltRain: return UltType::Rain;
    case CardId::UltBlackHole: return UltType::BlackHole;
    case CardId::UltNightmare: return UltType::Nightmare;
    default: return UltType::Pulse;
  }
}
bool IsUltCard(CardId id) { return id == CardId::UltRain || id == CardId::UltBlackHole || id == CardId::UltNightmare; }

bool Available(const Session& s, const OfferContext& ctx, const CardDef& d) {
  if (s.Card(d.id) >= d.maxStack) return false;
  if (d.id == CardId::Heal && ctx.hpFull) return false;
  if (IsUltCard(d.id) && UltRank(UltOf(d.id)) <= UltRank(s.prog.ult)) return false;  // 지금 필살기보다 높은 등급만
  return true;
}

Grade RollGrade(const Session& s, int stage) {
  const int luck = s.Card(CardId::Luck);
  float odds[4];
  for (int g = 0; g < 4; ++g) odds[g] = std::max(0.f, kGradeOdds[Clamp(stage, 0, 2)][g] + kLuckShift[g] * static_cast<float>(luck));
  float r = RandRange(0.f, odds[0] + odds[1] + odds[2] + odds[3]);
  for (int g = 0; g < 4; ++g) {
    if (r < odds[g]) return static_cast<Grade>(g);
    r -= odds[g];
  }
  return Grade::Common;
}

}  // namespace

const std::vector<CardDef>& AllCards() { return Table(); }
const CardDef& GetCard(CardId id) { return Table()[static_cast<size_t>(id)]; }

const char* GradeName(Grade g) {
  switch (g) {
    case Grade::Common: return "COMMON";
    case Grade::Rare: return "RARE";
    case Grade::Epic: return "EPIC";
    case Grade::Legend: return "LEGEND";
  }
  return "";
}

Color GradeColor(Grade g) {
  switch (g) {
    case Grade::Common: return Hex(0x9aa0b5);
    case Grade::Rare: return Hex(0x4fa3ff);
    case Grade::Epic: return Hex(0xe05cff);
    case Grade::Legend: return Hex(0xffc93c);
  }
  return Hex(0xffffff);
}

const char* CategoryName(CardCategory c) {
  switch (c) {
    case C::Attack: return "ATTACK";
    case C::Defense: return "DEFENSE";
    case C::Move: return "MOVE";
    case C::Dash: return "DASH";
    case C::Ult: return "ULT";
    case C::Utility: return "SPECIAL";
  }
  return "";
}

Color CategoryColor(CardCategory c) {
  switch (c) {
    case C::Attack: return Hex(0xff6a6a);
    case C::Defense: return Hex(0xff7aa8);
    case C::Move: return Hex(0x6fe08a);
    case C::Dash: return Hex(0x7ae0ff);
    case C::Ult: return Hex(0xc58bff);
    case C::Utility: return Hex(0xffe27a);
  }
  return Hex(0xffffff);
}

// 등급을 먼저 뽑고 그 등급 안에서 고른다. 그 등급에 남은 카드가 없으면 한 단계 아래(일반이면 위)로.
// 같은 분류는 최대 2장.
std::vector<CardId> DrawOffer(const Session& s, const OfferContext& ctx, int count) {
  std::vector<CardId> out;
  int perCat[6] = {};
  auto candidates = [&](Grade g) {
    std::vector<CardId> c;
    for (const CardDef& d : Table()) {
      if (d.grade != g || !Available(s, ctx, d)) continue;
      if (std::find(out.begin(), out.end(), d.id) != out.end()) continue;
      if (perCat[static_cast<int>(d.category)] >= 2) continue;
      c.push_back(d.id);
    }
    return c;
  };
  for (int slot = 0; slot < count; ++slot) {
    const Grade rolled = RollGrade(s, ctx.stage);
    // rolled 부터 아래로, 그다음 위로 찾는다
    int order[4], n = 0;
    for (int g = static_cast<int>(rolled); g >= 0; --g) order[n++] = g;
    for (int g = static_cast<int>(rolled) + 1; g < 4; ++g) order[n++] = g;
    for (int k = 0; k < 4; ++k) {
      const std::vector<CardId> c = candidates(static_cast<Grade>(order[k]));
      if (c.empty()) continue;
      const CardId pick = c[static_cast<size_t>(RandInt(0, static_cast<int>(c.size()) - 1))];
      out.push_back(pick);
      ++perCat[static_cast<int>(GetCard(pick).category)];
      break;
    }
  }
  if (ctx.guaranteeRare && !out.empty() &&
      std::all_of(out.begin(), out.end(), [](CardId id) { return GetCard(id).grade == Grade::Common; })) {
    // 일반만 나왔으면 첫 장을 희귀 이상으로 바꾼다
    --perCat[static_cast<int>(GetCard(out[0]).category)];
    const CardId old = out[0];
    out.erase(out.begin());
    for (int g = 1; g < 4; ++g) {
      std::vector<CardId> c = candidates(static_cast<Grade>(g));
      if (c.empty()) continue;
      out.insert(out.begin(), c[static_cast<size_t>(RandInt(0, static_cast<int>(c.size()) - 1))]);
      break;
    }
    if (static_cast<int>(out.size()) < count) out.insert(out.begin(), old);  // 희귀 이상이 하나도 안 남은 경우
  }
  return out;
}

int OfferSize(const Session& s) { return s.Has(CardId::MoreChoice) ? 4 : 3; }

void ApplyCard(Session& s, CardId id, Player* player) {
  const CardDef& d = GetCard(id);
  if (d.kind != CardKind::Consumable) ++s.Card(id);
  DebugLog("[card] got %s (Lv.%d)", d.name, s.Card(id));
  switch (id) {
    case CardId::MaxHeart:
      if (player) player->SetMaxHp(stats::MaxHp(s), 1);
      break;
    case CardId::Heal:
      if (player) player->Heal(2);
      break;
    case CardId::ExtraLife: ++s.lives; break;
    case CardId::Shield:
      if (player) player->GiveShield();
      break;
    case CardId::UltRain:
    case CardId::UltBlackHole:
    case CardId::UltNightmare: s.prog.ult = UltOf(id); break;
    default: break;
  }
}

int XpToNext(int level) { return 6 + 3 * (std::max(1, level) - 1); }

CardId FindCardByName(const char* name) {
  std::string want;
  for (const char* p = name; *p; ++p) {
    if (*p != ' ' && *p != '_' && *p != '-') want += static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
  }
  for (const CardDef& d : Table()) {
    std::string n;
    for (const char* p = d.name; *p; ++p) {
      if (*p != ' ' && *p != '-') n += static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
    }
    if (n == want) return d.id;
  }
  return CardId::Count;
}

}  // namespace gengar

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
      // 공격력: 희귀, 같은 등급의 다른 카드와 같은 비중 1 (비중 4 -> 2 로도 자주 보였다)
      {CardId::AttackUp, "ATTACK UP", "DAMAGE +3", G::Rare, C::Attack, K::Stat, 5},
      {CardId::FireRate, "FIRE RATE", "SHOOT FASTER", G::Common, C::Attack, K::Stat, 4},
      {CardId::RangeUp, "RANGE UP", "SHOTS GO FARTHER", G::Common, C::Attack, K::Stat, 3},
      {CardId::ShotSpeed, "SHOT SPEED", "SHOTS FLY FASTER", G::Common, C::Attack, K::Stat, 2},
      {CardId::Knockback, "KNOCKBACK", "HITS PUSH FOES BACK", G::Common, C::Attack, K::Stat, 2},
      {CardId::Critical, "CRITICAL", "SOMETIMES 2X DAMAGE", G::Rare, C::Attack, K::Stat, 3},
      {CardId::Pierce, "PIERCE", "SHOTS GO THROUGH 1 MORE FOE", G::Rare, C::Attack, K::SpecialStack, 2},
      {CardId::BackShot, "BACK SHOT", "ALSO SHOOT BEHIND YOU", G::Rare, C::Attack, K::Special, 1},
      {CardId::Execute, "EXECUTE", "MORE DAMAGE TO WEAK FOES", G::Rare, C::Attack, K::Special, 1},
      {CardId::MultiShot, "MULTI SHOT", "+1 SHOT", G::Epic, C::Attack, K::SpecialStack, 2},
      {CardId::Bounce, "BOUNCE", "SHOTS BOUNCE OFF FOES+WALLS", G::Rare, C::Attack, K::SpecialStack, 2},
      {CardId::BurstShot, "BURST SHOT", "SHOTS EXPLODE ON HIT", G::Epic, C::Attack, K::Special, 1},
      {CardId::Curse, "CURSE", "HIT FOES TAKE 2X DAMAGE FOR 2S", G::Rare, C::Attack, K::Special, 1},
      {CardId::Homing, "HOMING", "SHOTS CHASE FOES. DAMAGE -3", G::Epic, C::Attack, K::Special, 1},
      {CardId::Split, "SPLIT", "SPLIT IN 3 ON HIT OR WALL", G::Legend, C::Attack, K::Special, 1},
      {CardId::ChargeShot, "CHARGE SHOT", "HOLD Z LONGER: UP TO 10X", G::Epic, C::Attack, K::Special, 1},
      // 0.4.2: 공의 모양과 쏘는 방식을 바꾸는 카드. 혈사포는 차지샷·레이저와 함께 나오지 않는다
      {CardId::Brimstone, "BRIMSTONE", "HOLD Z: BLOOD LASER", G::Legend, C::Attack, K::Special, 1},
      {CardId::Laser, "LASER", "SHOTS BECOME FAST LASERS", G::Epic, C::Attack, K::Special, 1},
      {CardId::Boomerang, "BOOMERANG", "SHOTS COME BACK TO YOU", G::Rare, C::Attack, K::Special, 1},
      {CardId::WaveShot, "WAVE SHOT", "SHOTS WIGGLE IN A WAVE", G::Rare, C::Attack, K::Special, 1},
      {CardId::Scatter, "SCATTER", "+2 SHOTS, SHORT RANGE", G::Rare, C::Attack, K::Special, 1},
      {CardId::HeavyShot, "HEAVY SHOT", "BIG SLOW HEAVY SHOTS", G::Rare, C::Attack, K::Special, 1},
      {CardId::Poison, "POISON", "HITS POISON FOES", G::Rare, C::Attack, K::Special, 1},
      {CardId::Frost, "FROST", "HITS SLOW FOES DOWN", G::Rare, C::Attack, K::Special, 1},
      {CardId::Chain, "CHAIN BOLT", "HITS ZAP A NEARBY FOE", G::Epic, C::Attack, K::Special, 1},
      {CardId::Orbit, "SHADOW ORBIT", "2 ORBS CIRCLE YOU", G::Epic, C::Attack, K::SpecialStack, 2},

      {CardId::MaxHeart, "MAX HEART", "+1 MAX HEART", G::Common, C::Defense, K::Stat, 4},
      {CardId::Heal, "HEAL", "HEAL 2 HEARTS", G::Common, C::Defense, K::Consumable, 99},
      {CardId::LongGuard, "LONG GUARD", "LONGER SAFE TIME AFTER A HIT", G::Rare, C::Defense, K::Stat, 2},
      {CardId::SoulDrain, "SOUL DRAIN", "HEAL 1 EVERY 12 KILLS", G::Rare, C::Defense, K::Special, 1},
      {CardId::Thorns, "THORNS", "FOES THAT TOUCH YOU GET HURT", G::Rare, C::Defense, K::Special, 1},
      {CardId::Shield, "SHIELD", "BLOCK 1 HIT EACH STAGE", G::Epic, C::Defense, K::Special, 1},
      {CardId::ExtraLife, "EXTRA LIFE", "+1 LIFE", G::Epic, C::Defense, K::SpecialStack, 3},

      {CardId::MoveSpeed, "MOVE SPEED", "RUN FASTER", G::Common, C::Move, K::Stat, 3},
      {CardId::JumpUp, "JUMP UP", "JUMP HIGHER", G::Common, C::Move, K::Stat, 2},
      {CardId::AirJump, "DOUBLE JUMP", "+1 JUMP IN THE AIR", G::Rare, C::Move, K::SpecialStack, 2},
      {CardId::Float, "FLOAT", "HOLD JUMP TO FALL SLOWLY", G::Rare, C::Move, K::Special, 1},
      // 0.4.7: 날개가 돋아 공중에서 점프를 누르고 있으면 난다 (비행 기운이 있는 동안)
      {CardId::Wings, "SHADOW WINGS", "HOLD JUMP IN THE AIR TO FLY", G::Legend, C::Move, K::Special, 1},

      {CardId::DashRange, "DASH RANGE", "DASH FARTHER", G::Common, C::Dash, K::Stat, 2},
      {CardId::DashRecharge, "DASH RECHARGE", "DASH REFILLS FASTER", G::Common, C::Dash, K::Stat, 2},
      {CardId::DashCharge, "DASH CHARGE", "+1 DASH IN A ROW", G::Rare, C::Dash, K::SpecialStack, 2},
      {CardId::AirDash, "AIR DASH", "+1 DASH IN THE AIR", G::Rare, C::Dash, K::Special, 1},
      {CardId::PhaseDash, "PHASE DASH", "NO DAMAGE WHILE DASHING", G::Epic, C::Dash, K::Special, 1},
      {CardId::ShadowRush, "SHADOW RUSH", "DASH HITS FOES", G::Epic, C::Dash, K::Special, 1},
      {CardId::Afterimage, "AFTERIMAGE", "DASH LEAVES A GHOST BOMB", G::Legend, C::Dash, K::Special, 1},

      {CardId::GaugeUp, "GAUGE UP", "ULT FILLS FASTER", G::Common, C::Ult, K::Stat, 3},
      {CardId::UltPower, "ULT POWER", "ULT DAMAGE UP", G::Rare, C::Ult, K::Stat, 3},
      {CardId::HeadStart, "HEAD START", "START STAGES WITH SOME ULT", G::Common, C::Ult, K::Special, 1},
      {CardId::UltRain, "SHADOW RAIN", "ULT: SHOTS RAIN ON FOES", G::Rare, C::Ult, K::Special, 1},
      {CardId::UltBlackHole, "BLACK HOLE", "ULT: BLACK HOLE PULLS FOES", G::Epic, C::Ult, K::Special, 1},
      {CardId::UltNightmare, "NIGHTMARE", "ULT: STOP TIME", G::Legend, C::Ult, K::Special, 1},

      {CardId::Magnet, "MAGNET", "PULL COINS FROM FARTHER", G::Common, C::Utility, K::Stat, 2},
      {CardId::XpBoost, "XP BOOST", "MORE XP", G::Common, C::Utility, K::Stat, 3},
      {CardId::ComboKeep, "COMBO KEEP", "COMBO LASTS LONGER", G::Common, C::Utility, K::Special, 1},
      {CardId::Greed, "GREED", "MORE COINS", G::Rare, C::Utility, K::Stat, 2},
      {CardId::Luck, "LUCK", "BETTER CARDS MORE OFTEN", G::Rare, C::Utility, K::Stat, 3},
      {CardId::Reroll, "REROLL", "1 FREE REROLL EACH STAGE", G::Epic, C::Utility, K::SpecialStack, 2},
      {CardId::LastStand, "LAST STAND", "STRONGER AT 1 HEART", G::Epic, C::Utility, K::Special, 1},
      {CardId::MoreChoice, "MORE CHOICE", "LEVEL UP SHOWS 4 CARDS", G::Epic, C::Utility, K::Special, 1},
      {CardId::Familiar, "FAMILIAR", "MINI GHOST FIRES YOUR ATTACK", G::Legend, C::Utility, K::SpecialStack, 3},
  };
  return table;
}

// 등급 확률 (스테이지 1 / 2 / 3). 행운 카드 1단계마다 일반 -5%p, 희귀 +2, 영웅 +2, 전설 +1.
// 0.4.3: 영웅·전설이 너무 자주 나와서 낮춤. 0.4.4: 전설 3 / 5 / 8%. 0.4.5: 스테이지 1 일반 50% 기준으로 다시 맞춤
constexpr float kGradeOdds[3][4] = {
    {0.50f, 0.36f, 0.11f, 0.03f},
    {0.43f, 0.39f, 0.13f, 0.05f},
    {0.36f, 0.41f, 0.15f, 0.08f},
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

// 함께 가질 수 없는 카드: 혈사포는 Z 를 모아 쏘는 방식이라 차지샷·레이저와 겹치지 않게 한다
bool Excluded(const Session& s, CardId id) {
  if (id == CardId::Brimstone) return s.Has(CardId::ChargeShot) || s.Has(CardId::Laser);
  if (id == CardId::ChargeShot || id == CardId::Laser) return s.Has(CardId::Brimstone);
  return false;
}

bool Available(const Session& s, const OfferContext& ctx, const CardDef& d) {
  if (s.Card(d.id) >= d.maxStack) return false;
  if (Excluded(s, d.id)) return false;
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

// 비중(weight)대로 하나 고른다
CardId PickWeighted(const std::vector<CardId>& c) {
  int total = 0;
  for (CardId id : c) total += Table()[static_cast<size_t>(id)].weight;
  int r = RandInt(0, total - 1);
  for (CardId id : c) {
    r -= Table()[static_cast<size_t>(id)].weight;
    if (r < 0) return id;
  }
  return c.back();
}

}  // namespace

// 자세한 설명. 수치는 data/PlayerStats.h 와 각 능력 코드의 값이다 (바꾸면 여기도 맞춘다).
const char* CardDetail(CardId id) {
  switch (id) {
    case CardId::AttackUp: return "SHADOW BALL DAMAGE +3 PER LEVEL (BASE 10, MAX 25). SHOTS ALSO GROW BIGGER AND FLY A LITTLE FARTHER.";
    case CardId::FireRate: return "TIME BETWEEN SHOTS -15% PER LEVEL.";
    case CardId::RangeUp: return "SHOT RANGE +20% PER LEVEL. (ATTACK UP ALSO ADDS A LITTLE RANGE; SCATTER CUTS IT TO 55%.)";
    case CardId::ShotSpeed: return "SHOTS FLY 20% FASTER PER LEVEL.";
    case CardId::Knockback: return "HIT FOES ARE PUSHED BACK HARDER EACH LEVEL.";
    case CardId::Critical: return "15% CHANCE PER LEVEL FOR A HIT TO DEAL 2X DAMAGE. WORKS ON SPLITS, BURSTS AND DASH HITS.";
    case CardId::Pierce: return "EACH SHOT PASSES THROUGH 1 MORE FOE PER LEVEL BEFORE IT BREAKS.";
    case CardId::BackShot: return "EVERY VOLLEY ALSO FIRES THE SAME SHOTS BEHIND YOU AT 50% DAMAGE.";
    case CardId::Execute: return "FOES BELOW 30% HP TAKE 50% MORE DAMAGE. STACKS WITH CURSE.";
    case CardId::MultiShot: return "+1 SHOT PER LEVEL IN A SMALL FAN (UP TO 3). EVERY SHOT KEEPS ALL YOUR SHOT SKILLS.";
    case CardId::Bounce: return "SHOTS BOUNCE OFF WALLS, FLOORS AND FOES (FOES SEND THEM IN A RANDOM WAY). 2 BOUNCES, 3 AT LV 2. RANGE X1.6.";
    case CardId::BurstShot: return "SHOTS EXPLODE ON HIT, DEALING 50% DAMAGE TO FOES NEARBY.";
    case CardId::Curse: return "A HIT FOE TAKES 2X DAMAGE FOR 2 SECONDS. THE SAME FOE CAN BE CURSED AGAIN AFTER 15 SECONDS.";
    case CardId::Homing: return "SHOTS CURVE TOWARD THE NEAREST FOE AHEAD. DAMAGE -3 (MIN 7).";
    case CardId::Split: return "ON A HIT OR A WALL, A SHOT SPLITS INTO 3 SMALL SHOTS (40% DAMAGE, LONG RANGE). A BOSS TAKES ALL 3.";
    case CardId::ChargeShot: return "HOLD Z TO CHARGE, RELEASE TO FIRE A PIERCING BALL: 0.5S 2X, 1S 4X, 1.75S 7X, 2.5S 10X DAMAGE.";
    case CardId::Brimstone: return "NO MORE NORMAL SHOTS. HOLD Z FOR 0.75S, RELEASE TO FIRE A THICK BLOOD LASER THAT PIERCES EVERYTHING FOR 0.5S (ABOUT 6X DAMAGE). GROWS HORNS.";
    case CardId::Laser: return "SHOTS FLY 2.6X FASTER AS THIN LASERS AND PIERCE EVERY FOE. DAMAGE 70%.";
    case CardId::Boomerang: return "AT HALF RANGE SHOTS TURN AROUND AND FLY BACK TO YOU, HITTING FOES AGAIN ON THE WAY.";
    case CardId::WaveShot: return "SHOTS SWING UP AND DOWN IN A WAVE, COVERING MORE SPACE.";
    case CardId::Scatter: return "+2 SHOTS IN A WIDE FAN. RANGE 55%, DAMAGE 80% PER SHOT.";
    case CardId::HeavyShot: return "SHOTS ARE 1.45X BIGGER AND DEAL 40% MORE DAMAGE, BUT YOU SHOOT 25% SLOWER.";
    case CardId::Poison: return "HIT FOES ARE POISONED FOR 3 SECONDS: 25% OF THE HIT EVERY 0.5S (ABOUT 1.5X MORE). WORKS ON THE BOSS.";
    case CardId::Frost: return "HIT FOES MOVE AT HALF SPEED FOR 2 SECONDS. THE BOSS IS NOT SLOWED.";
    case CardId::Chain: return "EACH HIT ZAPS THE NEAREST OTHER FOE WITHIN 170PX FOR 50% DAMAGE.";
    case CardId::Orbit: return "2 SHADOW ORBS CIRCLE YOU (4 AT LV 2). THEY HIT FOES FOR 60% DAMAGE AND BLOCK ENEMY SHOTS.";
    case CardId::MaxHeart: return "+1 MAX HEART PER LEVEL.";
    case CardId::Heal: return "HEAL 2 HEARTS RIGHT AWAY.";
    case CardId::LongGuard: return "SAFE TIME AFTER TAKING A HIT +50% PER LEVEL.";
    case CardId::SoulDrain: return "EVERY 12 KILLS, HEAL 1 HEART.";
    case CardId::Thorns: return "FOES THAT BUMP INTO YOU TAKE 30 DAMAGE.";
    case CardId::Shield: return "A SHADOW SHIELD BLOCKS THE FIRST HIT IN EACH STAGE.";
    case CardId::ExtraLife: return "+1 LIFE PER LEVEL.";
    case CardId::MoveSpeed: return "RUN SPEED +0.45 PER LEVEL.";
    case CardId::JumpUp: return "JUMP 8% HIGHER PER LEVEL.";
    case CardId::AirJump: return "DOUBLE JUMP: +1 EXTRA JUMP IN THE AIR PER LEVEL (TRIPLE JUMP AT LV 2).";
    case CardId::Float: return "HOLD JUMP WHILE FALLING TO DRIFT DOWN SLOWLY.";
    case CardId::Wings: return "GROW SHADOW WINGS. IN THE AIR, HOLD JUMP TO FLY UP. FLIGHT LASTS 2.5S (BAR UNDER YOU) AND REFILLS ON THE GROUND. WHEN IT RUNS OUT YOU GLIDE DOWN.";
    case CardId::DashRange: return "DASH 25% FARTHER PER LEVEL.";
    case CardId::DashRecharge: return "EMPTY DASH SLOTS REFILL 20% FASTER PER LEVEL.";
    case CardId::DashCharge: return "+1 DASH SLOT PER LEVEL, SO YOU CAN DASH AGAIN RIGHT AWAY. ALSO +1 DASH IN THE AIR.";
    case CardId::AirDash: return "+1 DASH IN THE AIR BEFORE YOU LAND.";
    case CardId::PhaseDash: return "WHILE DASHING YOU TAKE NO DAMAGE AND PASS THROUGH FOES. YOU GLOW GREEN.";
    case CardId::ShadowRush: return "DASHING SLASHES FOES IN FRONT AND IN YOUR PATH FOR 2X DAMAGE.";
    case CardId::Afterimage: return "EACH DASH LEAVES A GHOST THAT EXPLODES FOR 3X DAMAGE.";
    case CardId::GaugeUp: return "ULT GAUGE FILLS 20% FASTER PER LEVEL.";
    case CardId::UltPower: return "ULT DAMAGE +30% PER LEVEL.";
    case CardId::HeadStart: return "START EVERY STAGE WITH 30% ULT GAUGE.";
    case CardId::UltRain: return "ULT: FOR 3 SECONDS, 18 SHADOW SHOTS RAIN DOWN ON FOES ON SCREEN.";
    case CardId::UltBlackHole: return "ULT: A BLACK HOLE PULLS FOES IN FOR 4 SECONDS, HURTS THEM AND EATS ENEMY SHOTS.";
    case CardId::UltNightmare: return "ULT: STOP TIME FOR 4 SECONDS. YOU ARE SAFE AND DEAL 2X DAMAGE, THEN A FINAL BLAST HITS ALL.";
    case CardId::Magnet: return "PULL COINS FROM FARTHER AWAY EACH LEVEL.";
    case CardId::XpBoost: return "+20% XP PER LEVEL.";
    case CardId::ComboKeep: return "THE COMBO TIMER LASTS 4.5 SECONDS INSTEAD OF 3.";
    case CardId::Greed: return "+25% COINS PER LEVEL.";
    case CardId::Luck: return "EACH LEVEL MOVES 5% FROM COMMON CARDS TO RARE, EPIC AND LEGEND.";
    case CardId::Reroll: return "1 FREE REROLL PER LEVEL IN EACH STAGE (PRESS R). AFTER THAT, REROLLS COST COINS.";
    case CardId::LastStand: return "AT 1 HEART: SHOTS DEAL 50% MORE DAMAGE AND FIRE 30% FASTER.";
    case CardId::MoreChoice: return "LEVEL UPS, BOSS BONUSES AND SHOP DRAWS SHOW 4 CARDS.";
    case CardId::Familiar: return "EVERY 0.8S A MINI GHOST FIRES YOUR ATTACK AT THE NEAREST FOE: SAME NUMBER OF SHOTS AND SHOT SKILLS, 50% DAMAGE. UP TO 3 GHOSTS, FOLLOWING IN A LINE.";
    case CardId::Count: break;
  }
  return "";
}

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
      const CardId pick = PickWeighted(c);
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
      out.insert(out.begin(), PickWeighted(c));
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

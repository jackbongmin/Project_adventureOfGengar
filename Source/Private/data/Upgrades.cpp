#include "data/Upgrades.h"

#include <cstdio>

#include "data/PlayerStats.h"

namespace gengar {

const std::vector<UpgradeDef>& AllUpgrades() {
  static const std::vector<UpgradeDef> table = {
      {UpgradeId::Power, "ATTACK POWER", "MORE DAMAGE, BIGGER BALL", 4, {20, 35, 55, 80}},
      {UpgradeId::Rate, "FIRE RATE", "SHOOT FASTER", 3, {20, 35, 50}},
      {UpgradeId::Multi, "MULTI SHOT", "MORE BALLS PER SHOT", 2, {40, 80}},
      {UpgradeId::MaxHp, "MAX HEARTS", "+1 HEART", 4, {20, 30, 45, 60}},
      {UpgradeId::DoubleJump, "DOUBLE JUMP", "JUMP AGAIN IN MID-AIR", 1, {45}},
      {UpgradeId::Speed, "MOVE SPEED", "RUN FASTER", 3, {15, 25, 40}},
      {UpgradeId::Ult, "ULTIMATE POWER", "BIGGER ULT, FASTER GAUGE", 3, {30, 50, 75}},
      {UpgradeId::Life, "EXTRA LIFE", "+1 LIFE", 3, {40, 60, 80}},
  };
  return table;
}

int NextCost(const UpgradeDef& def, const Session& session) {
  const int lv = session.Level(def.id);
  if (lv >= def.maxLevel) return -1;
  return def.cost[static_cast<size_t>(lv)];
}

namespace {
std::string Fmt(const char* fmt, double a, double b) {
  char buf[64];
  std::snprintf(buf, sizeof(buf), fmt, a, b);
  return buf;
}
}  // namespace

UpgradePreview PreviewUpgrade(const UpgradeDef& def, const Session& s) {
  Session next = s;
  ++next.Level(def.id);
  switch (def.id) {
    case UpgradeId::Power:
      return {"ATTACK DAMAGE", Fmt("%.0f -> %.0f", stats::Damage(s), stats::Damage(next))};
    case UpgradeId::Rate:
      return {"SHOTS PER SECOND", Fmt("%.1f -> %.1f", 60.0 / stats::FireCooldown(s), 60.0 / stats::FireCooldown(next))};
    case UpgradeId::Multi:
      return {"BALLS PER SHOT", Fmt("%.0f -> %.0f", stats::ShotCount(s), stats::ShotCount(next))};
    case UpgradeId::MaxHp:
      return {"MAX HEARTS", Fmt("%.0f -> %.0f", stats::MaxHp(s), stats::MaxHp(next))};
    case UpgradeId::DoubleJump:
      return {"AIR JUMPS", Fmt("%.0f -> %.0f", stats::AirJumps(s), stats::AirJumps(next))};
    case UpgradeId::Speed:
      return {"MOVE SPEED", Fmt("%.1f -> %.1f", stats::MoveSpeed(s), stats::MoveSpeed(next))};
    case UpgradeId::Ult:
      return {"ULT DAMAGE", Fmt("%.0f -> %.0f", stats::UltDamage(s), stats::UltDamage(next))};
    case UpgradeId::Life:
      return {"LIVES", Fmt("%.0f -> %.0f", s.lives, s.lives + 1)};
    case UpgradeId::Count: break;
  }
  return {"", ""};
}

}  // namespace gengar

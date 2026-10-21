#include "data/Upgrades.h"

namespace gengar {

const std::vector<UpgradeDef>& AllUpgrades() {
  static const std::vector<UpgradeDef> table = {
      {UpgradeId::Power, "SHADOW POWER", "ATTACK +1, BIGGER BALL", 4, {20, 35, 55, 80}},
      {UpgradeId::Rate, "RAPID FIRE", "SHORTER SHOT DELAY", 3, {20, 35, 50}},
      {UpgradeId::Multi, "MULTI SHOT", "+1 SHADOW BALL", 2, {40, 80}},
      {UpgradeId::MaxHp, "MAX HEALTH", "+1 HEART", 4, {20, 30, 45, 60}},
      {UpgradeId::Jump, "FLOAT JUMP", "JUMP HIGHER", 3, {15, 25, 40}},
      {UpgradeId::Speed, "SWIFT STEP", "MOVE FASTER", 3, {15, 25, 40}},
      {UpgradeId::Ult, "VOID BURST", "STRONGER ULT, FASTER GAUGE", 3, {30, 50, 75}},
      {UpgradeId::Life, "EXTRA LIFE", "+1 LIFE", 3, {40, 60, 80}},
  };
  return table;
}

int NextCost(const UpgradeDef& def, const Session& session) {
  const int lv = session.Level(def.id);
  if (lv >= def.maxLevel) return -1;
  return def.cost[static_cast<size_t>(lv)];
}

}  // namespace gengar

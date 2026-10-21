#pragma once
// 상점 업그레이드 표. 새 업그레이드는 UpgradeId에 추가하고 Upgrades.cpp 표에 한 줄 + PlayerStats에 수식 추가.
#include <vector>

#include "core/Session.h"

namespace gengar {

struct UpgradeDef {
  UpgradeId id;
  const char* name;
  const char* desc;
  int maxLevel;
  std::vector<int> cost;  // 레벨별 가격 (cost[현재 레벨])
};

const std::vector<UpgradeDef>& AllUpgrades();  // 상점 표시 순서
// 다음 레벨 가격. 최대 레벨이면 -1.
int NextCost(const UpgradeDef& def, const Session& session);

}  // namespace gengar

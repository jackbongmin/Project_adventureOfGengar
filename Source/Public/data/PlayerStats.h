#pragma once
// 업그레이드 레벨 -> 실제 능력치 변환식. 밸런스는 여기서 조정한다.
#include <algorithm>

#include "core/Config.h"
#include "core/Session.h"

namespace gengar::stats {

inline int MaxHp(const Session& s) { return cfg::kBaseHearts + s.Level(UpgradeId::MaxHp); }
inline float Damage(const Session& s) { return 1.f + static_cast<float>(s.Level(UpgradeId::Power)); }
inline int ShotCount(const Session& s) { return 1 + s.Level(UpgradeId::Multi); }
inline int FireCooldown(const Session& s) { return std::max(9, 24 - s.Level(UpgradeId::Rate) * 5); }
// 사거리 = 속도 8px * 수명. 기본 약 290px, 최대 약 380px.
inline int ShotLife(const Session& s) { return 36 + s.Level(UpgradeId::Power) * 3; }
inline float ShotRadius(const Session& s) { return 7.f + static_cast<float>(s.Level(UpgradeId::Power)) * 1.3f; }
inline float JumpSpeed(const Session& s) { return 11.8f + static_cast<float>(s.Level(UpgradeId::Jump)) * 0.9f; }
inline float MoveSpeed(const Session& s) { return 3.4f + static_cast<float>(s.Level(UpgradeId::Speed)) * 0.45f; }
inline float UltDamage(const Session& s) {
  return 6.f + static_cast<float>(s.Level(UpgradeId::Ult)) * 3.f + static_cast<float>(s.Level(UpgradeId::Power)) * 2.f;
}
inline float UltBossDamage(const Session& s) { return 20.f + static_cast<float>(s.Level(UpgradeId::Ult)) * 8.f; }
inline float GaugeMultiplier(const Session& s) { return 1.f + static_cast<float>(s.Level(UpgradeId::Ult)) * 0.25f; }

}  // namespace gengar::stats

#pragma once
// 카드 단계 -> 실제 능력치 변환식. 밸런스는 여기서 조정한다. (기획서 카드 목록의 '1회당 수치')
#include <algorithm>
#include <cmath>

#include "core/Config.h"
#include "core/Session.h"

namespace gengar::stats {

inline float Lv(const Session& s, CardId id) { return static_cast<float>(s.Card(id)); }

// ---- 공격 ----
inline float Damage(const Session& s) { return 1.f + Lv(s, CardId::AttackUp); }
inline int ShotCount(const Session& s) { return 1 + s.Card(CardId::MultiShot); }
// 발사 간격: 단계마다 15% 짧아진다 (24 -> 20 -> 17 -> 15 -> 13 프레임)
inline int FireCooldown(const Session& s) {
  return std::max(6, static_cast<int>(std::lround(24.f * std::pow(0.85f, Lv(s, CardId::FireRate)))));
}
inline float ShotSpeed(const Session& s) { return 8.f * (1.f + 0.2f * Lv(s, CardId::ShotSpeed)); }
// 사거리 = 속도 x 수명. 기본 약 290px. 사거리 카드는 수명을 20%씩 늘린다.
inline int ShotLife(const Session& s) {
  return static_cast<int>((36.f + Lv(s, CardId::AttackUp) * 3.f) * (1.f + 0.2f * Lv(s, CardId::RangeUp)));
}
// 공격력 단계마다 커진다. 도트 배율(반지름/4 반올림)이 2 -> 2 -> 3 -> 3 -> 4 로 바뀌고, 둘레의 빛은 단계마다 커진다.
inline float ShotRadius(const Session& s) { return 7.f + std::min(4.f, Lv(s, CardId::AttackUp)) * 1.8f; }
inline float CritChance(const Session& s) { return 0.15f * Lv(s, CardId::Critical); }
inline int Pierce(const Session& s) { return s.Card(CardId::Pierce); }
inline int Bounces(const Session& s) { return s.Card(CardId::Bounce); }
inline float Knockback(const Session& s) { return 2.5f * Lv(s, CardId::Knockback); }  // 넉백 속도 (px/프레임)

// ---- 방어 ----
inline int MaxHp(const Session& s) { return cfg::kBaseHearts + s.Card(CardId::MaxHeart); }
inline int HurtInvuln(const Session& s) { return static_cast<int>(100.f * (1.f + 0.5f * Lv(s, CardId::LongGuard))); }

// ---- 이동 ----
inline float JumpSpeed(const Session& s) { return 11.8f * (1.f + 0.08f * Lv(s, CardId::JumpUp)); }
inline int AirJumps(const Session& s) { return s.Card(CardId::AirJump); }
inline float MoveSpeed(const Session& s) { return 3.4f + Lv(s, CardId::MoveSpeed) * 0.45f; }

// ---- 대시 ----
inline float DashSpeed(const Session& s) { return 11.f * (1.f + 0.25f * Lv(s, CardId::DashRange)); }  // 기본 약 110px
inline int DashCharges(const Session& s) { return 1 + s.Card(CardId::DashCharge); }
inline int DashRecharge(const Session& s) { return static_cast<int>(36.f * (1.f - 0.2f * Lv(s, CardId::DashRecharge))); }
inline int AirDashes(const Session& s) { return 1 + s.Card(CardId::AirDash); }

// ---- 필살기 ----
inline float UltMultiplier(const Session& s) { return 1.f + 0.3f * Lv(s, CardId::UltPower); }
inline float GaugeMultiplier(const Session& s) { return 1.f + 0.2f * Lv(s, CardId::GaugeUp); }
inline float UltDamage(const Session& s) { return (6.f + Lv(s, CardId::AttackUp) * 2.f) * UltMultiplier(s); }
inline float UltBossDamage(const Session& s) { return 20.f * UltMultiplier(s); }

// ---- 특수 ----
inline float XpMultiplier(const Session& s) { return 1.f + 0.2f * Lv(s, CardId::XpBoost); }
inline float CoinMultiplier(const Session& s) { return 1.f + 0.25f * Lv(s, CardId::Greed); }
inline float MagnetRadius(const Session& s) { return 22.f + 60.f * Lv(s, CardId::Magnet); }
inline int ComboFrames(const Session& s) { return s.Has(CardId::ComboKeep) ? 270 : 180; }
inline int Rerolls(const Session& s) { return s.Card(CardId::Reroll); }

// 보스 체력: 플레이어의 성장과 상관없이 고정이다.
inline int BossMaxHp(const Session&) { return cfg::kBossHp; }

// 플레이어 공격이 대상에게 들어갈 때의 배율: 처형자(빈사 +50%) x 저주(+25%)
inline float TargetMultiplier(const Session& s, float hpRatio, bool cursed) {
  float m = 1.f;
  if (s.Has(CardId::Execute) && hpRatio <= 0.3f) m *= 1.5f;
  if (cursed) m *= 1.25f;
  return m;
}

}  // namespace gengar::stats

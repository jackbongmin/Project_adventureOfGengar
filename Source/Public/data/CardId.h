#pragma once
// 로그라이크 카드 ID. 순서는 기획서(docs/design/roguelike_cards.xlsx)의 카드 목록과 같다.
// 카드를 추가하려면 여기에 ID 를 넣고 Cards.cpp 표에 한 줄, art/CardIcons.cpp 에 아이콘 한 줄을 추가한다.

namespace gengar {

enum class CardId {
  // 공격
  AttackUp, FireRate, RangeUp, ShotSpeed, Knockback, Critical, Pierce, BackShot, Execute,
  MultiShot, Bounce, BurstShot, Curse, Homing, Split, ChargeShot,
  Brimstone, Laser, Boomerang, WaveShot, Scatter, HeavyShot, Poison, Frost, Chain, Orbit,  // 0.4.2: 공 모양/메타 카드
  // 방어
  MaxHeart, Heal, LongGuard, SoulDrain, Thorns, Shield, ExtraLife,
  // 이동
  MoveSpeed, JumpUp, AirJump, Float, Wings,
  // 대시
  DashRange, DashRecharge, DashCharge, AirDash, PhaseDash, ShadowRush, Afterimage,
  // 필살기
  GaugeUp, UltPower, HeadStart, UltRain, UltBlackHole, UltNightmare,
  // 특수
  Magnet, XpBoost, ComboKeep, Greed, Luck, Reroll, LastStand, MoreChoice, Familiar,
  Count
};
constexpr int kCardCount = static_cast<int>(CardId::Count);

// 장착한 필살기. 높은 등급 필살기 카드를 고르면 교체된다.
enum class UltType { Pulse, Rain, BlackHole, Nightmare };

}  // namespace gengar

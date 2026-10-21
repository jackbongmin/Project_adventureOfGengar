#include "data/BossPatterns.h"

namespace gengar {

const std::vector<BossAttack>& AttackSequence(int phase) {
  using A = BossAttack;
  // 단계가 오를수록 원거리 탄막과 "예고 후 달려드는" 기술이 늘어난다.
  static const std::vector<BossAttack> seq[3] = {
      {A::Volley, A::Rain, A::Spiral, A::Pillar, A::Lunge, A::Sweep, A::Crossfire},
      {A::Volley, A::Homing, A::Beam, A::Lunge, A::Crossfire, A::Pillar, A::Summon, A::Dash, A::Sweep, A::Spiral, A::Blink},
      {A::Ring, A::Spiral, A::Beam, A::Lunge, A::Blink, A::Crossfire, A::Homing, A::Rain, A::Slam, A::Sweep, A::Dash, A::Lunge, A::Summon, A::Pillar, A::Beam},
  };
  if (phase < 0) phase = 0;
  if (phase > 2) phase = 2;
  return seq[phase];
}

const char* AttackName(BossAttack a) {
  switch (a) {
    case BossAttack::Volley: return "DARK VOLLEY";
    case BossAttack::Ring: return "VOID RING";
    case BossAttack::Summon: return "SUMMON";
    case BossAttack::Slam: return "SLAM";
    case BossAttack::Beam: return "VOID BEAM";
    case BossAttack::Rain: return "SHADOW RAIN";
    case BossAttack::Pillar: return "SHADOW PILLAR";
    case BossAttack::Dash: return "GROUND SWEEP";
    case BossAttack::Blink: return "BLINK";
    case BossAttack::Spiral: return "SPIRAL";
    case BossAttack::Homing: return "HOMING ORBS";
    case BossAttack::Lunge: return "LUNGE";
    case BossAttack::Crossfire: return "CROSSFIRE";
    case BossAttack::Sweep: return "VOID SWEEP";
  }
  return "";
}

}  // namespace gengar

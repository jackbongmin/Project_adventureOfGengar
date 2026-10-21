#include "data/BossPatterns.h"

namespace gengar {

const std::vector<BossAttack>& AttackSequence(int phase) {
  using A = BossAttack;
  static const std::vector<BossAttack> seq[3] = {
      {A::Volley, A::Pillar, A::Volley, A::Slam, A::Rain},
      {A::Volley, A::Rain, A::Summon, A::Slam, A::Beam, A::Dash, A::Pillar},
      {A::Ring, A::Beam, A::Blink, A::Rain, A::Slam, A::Pillar, A::Dash, A::Summon, A::Volley},
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
    case BossAttack::Dash: return "CHARGE";
    case BossAttack::Blink: return "BLINK";
  }
  return "";
}

}  // namespace gengar

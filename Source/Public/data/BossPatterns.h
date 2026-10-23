#pragma once
// 보스 공격 패턴 목록. 단계(phase)별 목록만 고치면 패턴 구성을 바꿀 수 있다. (보스는 이 중에서 무작위로 고른다)
#include <vector>

namespace gengar {

enum class BossAttack { Volley, Ring, Summon, Slam, Beam, Rain, Pillar, Dash, Blink, Spiral, Homing, Lunge, Crossfire, Sweep };

const std::vector<BossAttack>& AttackSequence(int phase);  // phase 0~2
const char* AttackName(BossAttack attack);

}  // namespace gengar

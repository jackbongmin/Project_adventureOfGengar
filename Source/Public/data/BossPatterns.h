#pragma once
// 보스 공격 패턴 순서표. 단계(phase)별 시퀀스만 고치면 패턴 구성을 바꿀 수 있다.
#include <vector>

namespace gengar {

enum class BossAttack { Volley, Ring, Summon, Slam, Beam, Rain, Pillar, Dash, Blink, Spiral, Homing, Lunge };

const std::vector<BossAttack>& AttackSequence(int phase);  // phase 0~2
const char* AttackName(BossAttack attack);

}  // namespace gengar

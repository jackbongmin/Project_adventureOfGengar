#pragma once
// 보스 상태 클래스들. 새 패턴을 추가하려면:
//  1) data/BossPatterns.h 의 BossAttack 에 항목 추가 + 시퀀스/이름 표 수정
//  2) 여기에 State<Boss> 파생 클래스를 만들고 MakeBossAttack 의 switch 에 등록
#include <memory>

#include "actor/boss/Boss.h"
#include "data/BossPatterns.h"
#include "state/StateMachine.h"

namespace gengar {

using BossState = State<Boss>;

std::unique_ptr<BossState> MakeBossEnterState();
std::unique_ptr<BossState> MakeBossFloatState();
std::unique_ptr<BossState> MakeBossDyingState();
std::unique_ptr<BossState> MakeBossAttackState(BossAttack attack);

}  // namespace gengar

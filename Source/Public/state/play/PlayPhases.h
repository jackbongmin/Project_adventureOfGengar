#pragma once
// PlayMode의 진행 단계 상태들.
//  Playing  -> (일시정지, PausedPhase.cpp) Paused -> Playing / 다시 시작 / 메인 메뉴
//  Playing  -> (플레이어 사망) PlayerDead -> 재도전/게임오버
//  Playing  -> (레벨업 / 보스 체력 단계 전환) CardPick -> Playing
//  Playing  -> (Tab, StatusPhase.cpp) Status -> Playing
//  Playing  -> (깃발) StageClear -> 상점,  (보스 격파 후 보물 모자 획득) HatGet -> 엔딩
#include <memory>

#include "mode/PlayMode.h"
#include "state/StateMachine.h"

namespace gengar {

using PlayPhase = State<PlayMode>;

std::unique_ptr<PlayPhase> MakePlayingPhase();
std::unique_ptr<PlayPhase> MakePausedPhase();
std::unique_ptr<PlayPhase> MakePlayerDeadPhase();
std::unique_ptr<PlayPhase> MakeCardPickPhase();
std::unique_ptr<PlayPhase> MakeStageClearPhase();
std::unique_ptr<PlayPhase> MakeHatGetPhase();
std::unique_ptr<PlayPhase> MakeStatusPhase();

}  // namespace gengar

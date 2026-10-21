#pragma once
// 스테이지 플레이 모드. World를 소유하고, 진행 단계(플레이/일시정지/사망/클리어)는
// StateMachine<PlayMode> 의 상태 객체(state/play/PlayPhases.h)로 표현한다.
#include <memory>

#include "mode/GameMode.h"
#include "state/StateMachine.h"
#include "world/World.h"

namespace gengar {

class PlayMode : public GameMode {
 public:
  PlayMode(Game& game, int stageIndex);

  void Update(Game& game) override;
  void Render(Graphics& gfx) override;

  Game& GetGame() { return game_; }
  World& GetWorld() { return *world_; }
  StateMachine<PlayMode>& Phase() { return phase_; }

 private:
  Game& game_;
  std::unique_ptr<World> world_;
  StateMachine<PlayMode> phase_;
};

}  // namespace gengar

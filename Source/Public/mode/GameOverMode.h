#pragma once
#include "mode/GameMode.h"

namespace gengar {

class GameOverMode : public GameMode {
 public:
  void Update(Game& game) override;
  void Render(Graphics& gfx) override;

 private:
  int frame_ = 0;
  int score_ = 0;
};

}  // namespace gengar

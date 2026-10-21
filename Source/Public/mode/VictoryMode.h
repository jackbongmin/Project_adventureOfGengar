#pragma once
// 엔딩 시네마틱: 되찾은 보물 모자를 쓰고 행복해하는 팬텀 -> THE END (+ 최종 점수).
// Enter/Z = 다음 장면, C 꾹 = THE END 화면으로 건너뛰기.
#include "mode/Cutscene.h"
#include "mode/GameMode.h"

namespace gengar {

class VictoryMode : public GameMode {
 public:
  void Update(Game& game) override;
  void Render(Graphics& gfx) override;

 private:
  bool Crossed(int frame) const { return prev_ < frame && t_ >= frame; }

  int t_ = 0;
  int prev_ = -1;
  int score_ = 0;
  SkipHold skip_;
};

}  // namespace gengar

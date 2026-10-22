#pragma once
// 오프닝 시네마틱: VOIDMAW가 팬텀의 보물 모자를 훔쳐 달아난다. 끝나면 새 게임을 시작한다.
// Enter/Z = 다음 장면 (대사는 자동으로 넘어가지 않고 버튼을 눌러야 넘어간다), C 꾹 = 전체 건너뛰기.
#include "mode/Cutscene.h"
#include "mode/GameMode.h"

namespace gengar {

class IntroMode : public GameMode {
 public:
  void Update(Game& game) override;
  void Render(Graphics& gfx) override;

 private:
  bool Crossed(int frame) const { return prev_ < frame && t_ >= frame; }  // 이번 스텝에 frame을 지났는가 (장면 건너뛰기에도 안전)

  int t_ = 0;
  int prev_ = -1;
  int blink_ = 0;  // 대사에서 멈춰 있는 동안에도 흐르는 프레임 ("다음" 화살표 깜빡임용)
  SkipHold skip_;
};

}  // namespace gengar

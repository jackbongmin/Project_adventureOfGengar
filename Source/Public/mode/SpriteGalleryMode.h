#pragma once
// 개발용 화면: 게임에 쓰이는 모든 도트 스프라이트를 한눈에 본다. (타이틀에서 G 키, Esc/Enter로 돌아가기)
// 스프라이트를 고친 뒤 실제 게임을 돌리지 않고도 모양과 팔레트를 확인할 수 있다.
#include "mode/GameMode.h"

namespace gengar {

class SpriteGalleryMode : public GameMode {
 public:
  void Update(Game& game) override;
  void Render(Graphics& gfx) override;

 private:
  int frame_ = 0;
};

}  // namespace gengar

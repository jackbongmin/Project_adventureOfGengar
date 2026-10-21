#pragma once
// 스테이지 사이 상점. 코인으로 업그레이드를 구매하고 다음 스테이지로 출발한다.
#include "mode/GameMode.h"

namespace gengar {

class ShopMode : public GameMode {
 public:
  explicit ShopMode(Game& game) : game_(game) {}
  void Update(Game& game) override;
  void Render(Graphics& gfx) override;

 private:
  Game& game_;
  int selected_ = 0;
  int errorTimer_ = 0;
  int frame_ = 0;
};

}  // namespace gengar

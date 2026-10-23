#pragma once
// 타이틀 + 메인 메뉴. 한 모드 안에서 화면(메인 / 스테이지 선택 / 조작법 / 설정)을 바꿔 가며 보여 준다.
#include "mode/GameMode.h"

namespace gengar {

class TitleMode : public GameMode {
 public:
  void Update(Game& game) override;
  void Render(Graphics& gfx) override;

 private:
  enum class Screen { Main, Stages, Controls, Settings };

  void UpdateMain(Game& game);
  void UpdateStages(Game& game);
  void UpdateSettings(Game& game);
  void Go(Screen s) { screen_ = s; cursor_ = 0; }

  Screen screen_ = Screen::Main;
  int cursor_ = 0;
  int frame_ = 0;
};

}  // namespace gengar

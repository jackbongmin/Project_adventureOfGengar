#pragma once
// 게임 모드 = 화면 단위 흐름(타이틀, 플레이, 상점, 게임오버, 엔딩).
// Game이 현재 모드 하나만 들고 Update/Render를 위임한다. 모드 전환은 Game::GoXxx()로 한다.

namespace gengar {

class Game;
class Graphics;

class GameMode {
 public:
  virtual ~GameMode() = default;
  virtual void Update(Game& game) = 0;
  virtual void Render(Graphics& gfx) = 0;
};

}  // namespace gengar

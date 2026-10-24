#pragma once
// 스테이지 사이 상점. 코인 50개로 카드 뽑기 기회를 사고(희귀 이상 1장 보장), 다음 스테이지로 출발한다.
// 왼쪽에는 지금 캐릭터의 능력치와 얻은 카드를 보여 준다.
#include <string>

#include "mode/GameMode.h"
#include "ui/CardPicker.h"

namespace gengar {

class ShopMode : public GameMode {
 public:
  explicit ShopMode(Game& game) : game_(game) {}
  void Update(Game& game) override;
  void Render(Graphics& gfx) override;

 private:
  void StartDraw(Game& game);

  Game& game_;
  int selected_ = 0;      // 0 = 카드 뽑기, 1 = 출발
  int errorTimer_ = 0;    // 코인이 모자랄 때 뽑기 칸을 흔든다
  int flashTimer_ = 0;    // 방금 카드를 얻음 (패널 반짝임)
  int draws_ = 0;         // 이번 상점에서 뽑은 횟수
  int leaveTimer_ = -1;   // 출발을 누른 뒤 화면이 검게 닫히는 중 (-1 = 아님)
  std::string nextName_;  // 다음 스테이지 이름 (처음 그릴 때 한 번 읽는다)
  int frame_ = 0;
  bool picking_ = false;
  CardPicker picker_;
};

}  // namespace gengar

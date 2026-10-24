#pragma once
// 스테이지 사이 상점. 오른쪽 세 칸: 카드 뽑기(희귀 이상 1장 보장, 뽑을 때마다 비싸진다) / 카드 강화 / 다음 스테이지로 출발.
// 카드 강화: 얻은 카드 중 단계를 더 올릴 수 있는 것(능력치, 특수(중첩))을 골라 코인으로 1단계 올린다. 등급이 높을수록 비싸다.
// 왼쪽에는 지금 캐릭터의 능력치와 얻은 카드를 보여 준다.
#include <string>
#include <vector>

#include "data/Cards.h"
#include "mode/GameMode.h"
#include "ui/CardPicker.h"

namespace gengar {

class ShopMode : public GameMode {
 public:
  explicit ShopMode(Game& game) : game_(game) {}
  void Update(Game& game) override;
  void Render(Graphics& gfx) override;

  // 뽑기 한 번 가격: 50 에서 시작해 이번 판에 뽑을 때마다 +25 (상한 없음). 다음 상점에도 이어진다
  int DrawCost() const;
  // 강화 가격: 등급 기본값(일반 60 / 희귀 90 / 영웅 140 / 전설 200) + 지금 단계마다 20
  static int UpgradeCost(const Session& s, CardId id);
  static std::vector<CardId> Upgradable(const Session& s);  // 단계를 더 올릴 수 있는 얻은 카드

 private:
  enum Item { kDraw, kUpgrade, kDepart, kItemCount };

  void StartDraw(Game& game);
  void UpdateUpgrade(Game& game);
  void RenderUpgrade(Graphics& g, const Session& s) const;

  Game& game_;
  int selected_ = kDraw;
  int errorTimer_ = 0;    // 코인이 모자랄 때 고른 칸을 흔든다
  int flashTimer_ = 0;    // 방금 카드를 얻거나 올림 (패널 반짝임)
  int draws_ = 0;         // 이번 상점에서 뽑은 횟수
  int frame_ = 0;
  int leaveTimer_ = -1;   // 출발을 누른 뒤 화면이 검게 닫히는 중 (-1 = 아님)
  std::string nextName_;  // 다음 스테이지 이름 (처음 그릴 때 한 번 읽는다)
  bool picking_ = false;
  CardPicker picker_;
  bool upgrading_ = false;  // 강화 화면이 열려 있다
  int upCursor_ = 0;
  int upFlash_ = 0;         // 방금 올린 줄이 반짝이는 남은 프레임
  CardId upFlashId_ = CardId::Count;
};

}  // namespace gengar

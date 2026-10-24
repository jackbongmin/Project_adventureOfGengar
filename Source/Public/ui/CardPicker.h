#pragma once
// 카드 3장(또는 4장) 중 1장을 고르는 화면. 레벨업, 보스 단계 전환, 상점 뽑기가 함께 쓴다.
// 카드 테두리는 등급마다 다르다: 일반(회색) / 희귀(파랑 + 모서리 도트) / 영웅(마젠타 + 테두리를 도는 빛)
// / 전설(금색 + 반짝임, 열릴 때 흰 플래시).
#include <string>
#include <vector>

#include "data/Cards.h"

namespace gengar {

class Graphics;
class Input;
class Audio;

class CardPicker {
 public:
  enum class Result { None, Picked, Reroll };

  // freeRerolls = 남은 무료 다시 뽑기(리롤 카드). rerollCost >= 0 이면 무료가 없을 때 코인을 내고 다시 뽑을 수 있다
  // (coins = 지금 가진 코인). rerollCost < 0 이면 코인 다시 뽑기 없음 (상점).
  void Open(std::vector<CardId> offer, std::string title, int freeRerolls, int rerollCost = -1, int coins = 0);
  // bot = 자동 플레이: 잠시 뒤 등급이 가장 높은 카드를 고른다.
  Result Update(Input& in, Audio& audio, bool bot);
  CardId Choice() const { return offer_.empty() ? CardId::Count : offer_[static_cast<size_t>(cursor_)]; }
  bool CanReroll() const { return freeRerolls_ > 0 || (rerollCost_ >= 0 && coins_ >= rerollCost_); }
  bool Empty() const { return offer_.empty(); }
  void Render(Graphics& g, const Session& s) const;

 private:
  static constexpr int kArmFrames = 30;  // 카드가 펼쳐지는 동안은 입력을 받지 않는다 (공격 키 Z 가 확정 키와 같아서)

  std::vector<CardId> offer_;
  std::string title_;
  int freeRerolls_ = 0;
  int rerollCost_ = -1;
  int coins_ = 0;
  int cursor_ = 0;
  int t_ = 0;
};

// 카드 한 장. (x, y) = 왼쪽 위. t = 펼쳐진 뒤 지난 프레임 (반짝임/플래시 연출).
void DrawCard(Graphics& g, const CardDef& def, const Session& s, float x, float y, float w, float h, bool selected, int t);
// 등급 색 테두리를 두른 작은 아이콘 칸 (캐릭터 패널, HUD). size = 칸 한 변 (px)
void DrawCardIconSlot(Graphics& g, CardId id, float x, float y, float size, int stack);

}  // namespace gengar

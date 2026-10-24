#pragma once
// 로그라이크 카드 표와 뽑기 규칙. 기획서: docs/design/roguelike_cards.xlsx
//  - 레벨업(경험치), 보스 체력 단계 전환, 상점 뽑기(50코인)에서 카드 3장 중 1장을 고른다.
//  - 능력치 카드는 최대 단계까지 다시 나오고, 특수 카드는 한 번만 나온다. 특수(중첩)는 개수로 쌓인다.
//  - 서로 다른 특수 능력은 모두 함께 적용된다.
#include <vector>

#include "core/Session.h"
#include "core/Types.h"

namespace gengar {

class Player;

inline constexpr int kCardDrawCost = 50;  // 상점 카드 뽑기 첫 가격 (그 판에서 뽑을 때마다 +25, ShopMode::DrawCost)

enum class Grade { Common, Rare, Epic, Legend };
enum class CardCategory { Attack, Defense, Move, Dash, Ult, Utility };
enum class CardKind { Stat, Special, SpecialStack, Consumable };

struct CardDef {
  CardId id;
  const char* name;   // 게임 표기 (픽셀 폰트: 영문 대문자/숫자)
  const char* desc;   // 효과 설명. 카드 폭에 맞춰 단어 단위로 줄바꿈한다.
  Grade grade;
  CardCategory category;
  CardKind kind;
  int maxStack;       // 최대 단계 (99 = 제한 없음)
  int weight = 1;     // 같은 등급 안에서 뽑히는 비중 (자주 나와야 하는 카드는 크게)
};

const CardDef& GetCard(CardId id);
// 능력치 창에서 마우스를 올리면 보여 주는 자세한 설명 (수치 포함, 영문 대문자)
const char* CardDetail(CardId id);
const std::vector<CardDef>& AllCards();

const char* GradeName(Grade g);
Color GradeColor(Grade g);
const char* CategoryName(CardCategory c);
Color CategoryColor(CardCategory c);

// 뽑기 상황. 확률표는 스테이지별(0~2)로 다르다. 보스전(스테이지 3) 카드는 stage = 2.
struct OfferContext {
  int stage = 0;
  bool hpFull = true;          // 회복 카드는 하트가 가득이 아닐 때만 나온다
  bool guaranteeRare = false;  // 상점 뽑기: 희귀 이상 1장 보장
};
// 카드 count장(보통 3, 넓은 선택 카드면 4)을 서로 다르게 뽑는다. 남은 카드가 모자라면 더 적게 나온다.
std::vector<CardId> DrawOffer(const Session& s, const OfferContext& ctx, int count);
// 이번 레벨업 화면에 몇 장을 보여 줄지 (넓은 선택 카드)
int OfferSize(const Session& s);

// 카드를 얻는다. player 가 있으면(스테이지 중) 하트/보호막처럼 즉시 바뀌는 것도 반영한다.
void ApplyCard(Session& s, CardId id, Player* player);

// 레벨업에 필요한 경험치: 6 + 3 x (레벨 - 1)
int XpToNext(int level);

// 디버그 옵션 cards=homing,split 처럼 이름(게임 표기에서 공백을 뺀 소문자)으로 찾는다. 없으면 Count.
CardId FindCardByName(const char* name);

}  // namespace gengar

#pragma once
// 한 판(run) 동안 유지되는 진행 데이터: 코인, 점수, 목숨, 카드, 경험치, 필살기 게이지.
#include "core/Config.h"
#include "data/CardId.h"

namespace gengar {

// 카드와 레벨: 사망 후 재도전하면 스테이지 시작 시점으로 되돌린다 (코인과 같은 규칙)
struct Progress {
  int cards[kCardCount] = {};  // 카드별로 얻은 단계 (0 = 없음)
  int level = 1;
  int xp = 0;                  // 이번 레벨에서 모은 경험치
  UltType ult = UltType::Pulse;
};

struct Session {
  int stage = 0;
  int lives = cfg::kStartLives;
  int coins = 0;
  float coinFrac = 0.f;  // 탐욕 카드의 소수점 코인 (1이 넘으면 코인 1개)
  int score = 0;
  float gauge = 0.f;  // 필살기 게이지 0~100
  Progress prog;

  // 스테이지 시작 시점 스냅샷 (사망 후 재도전 시 복구)
  int snapCoins = 0;
  int snapScore = 0;
  Progress snapProg;

  void Reset() { *this = Session{}; }
  int Card(CardId id) const { return prog.cards[static_cast<int>(id)]; }
  int& Card(CardId id) { return prog.cards[static_cast<int>(id)]; }
  bool Has(CardId id) const { return Card(id) > 0; }
  void TakeSnapshot() { snapCoins = coins; snapScore = score; snapProg = prog; }
  void RestoreSnapshot() { coins = snapCoins; score = snapScore; prog = snapProg; }
};

}  // namespace gengar

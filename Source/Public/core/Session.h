#pragma once
// 한 판(run) 동안 유지되는 진행 데이터: 코인, 점수, 목숨, 업그레이드 레벨, 필살기 게이지.
#include "core/Config.h"

namespace gengar {

enum class UpgradeId { Power, Rate, Multi, MaxHp, Jump, Speed, Ult, Life, Count };
constexpr int kUpgradeCount = static_cast<int>(UpgradeId::Count);

struct Session {
  int stage = 0;
  int lives = cfg::kStartLives;
  int coins = 0;
  int score = 0;
  float gauge = 0.f;  // 필살기 게이지 0~100
  int levels[kUpgradeCount] = {};

  // 스테이지 시작 시점 스냅샷 (사망 후 재도전 시 복구)
  int snapCoins = 0;
  int snapScore = 0;

  void Reset() { *this = Session{}; }
  int Level(UpgradeId id) const { return levels[static_cast<int>(id)]; }
  int& Level(UpgradeId id) { return levels[static_cast<int>(id)]; }
  void TakeSnapshot() { snapCoins = coins; snapScore = score; }
  void RestoreSnapshot() { coins = snapCoins; score = snapScore; }
};

}  // namespace gengar

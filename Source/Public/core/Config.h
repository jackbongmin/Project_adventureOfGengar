#pragma once
// 게임 전역 상수. 밸런스 수치는 한 곳에서 조정한다.

namespace gengar::cfg {

inline constexpr int kScreenW = 960;
inline constexpr int kScreenH = 544;
inline constexpr int kTile = 32;
inline constexpr int kRows = 17;
inline constexpr int kGroundRow = 15;

inline constexpr float kFixedStep = 1.0f / 60.0f;  // 로직은 60Hz 고정 (속도 단위: px/프레임)

inline constexpr float kGravity = 0.55f;
inline constexpr float kMaxFall = 12.0f;

inline constexpr int kStageCount = 3;
inline constexpr int kBossStage = 2;  // 마지막 스테이지 = 보스전
inline constexpr int kBossHp = 300;  // 기본 체력. 공격 업그레이드를 많이 할수록 늘어난다 (stats::BossMaxHp)

inline constexpr int kCrumbleFrames = 50;    // 밟고 약 0.8초 뒤 추락
inline constexpr int kCrumbleRespawn = 200;  // 약 3.3초 뒤 복구

inline constexpr int kStartLives = 3;
inline constexpr int kBaseHearts = 3;

}  // namespace gengar::cfg

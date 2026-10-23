#pragma once
// 애플리케이션 루트: 서비스(그래픽/입력/오디오/세션)를 소유하고,
// 고정 60Hz 스텝으로 현재 GameMode를 갱신/렌더링한다. 모드 전환은 여기의 GoXxx()로만 한다.
#include <cstdint>
#include <memory>

#include "core/GameContext.h"
#include "core/Session.h"
#include "mode/GameMode.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "platform/Input.h"

namespace gengar {

class Game {
 public:
  Game();
  ~Game();

  bool Init();
  void Run();
  void Frame();  // 메인 루프 1회 (웹 빌드의 콜백에서 호출)
  void Quit() { running_ = false; }  // 네이티브 전용. 웹에서는 메뉴에 표시하지 않는다.

  Graphics& Gfx() { return gfx_; }
  Input& In() { return input_; }
  Audio& Snd() { return audio_; }
  Session& Sess() { return session_; }
  GameContext& Context() { return ctx_; }

  // ---- 화면(모드) 전환 ----
  void GoTitle();
  void GoIntro();                     // 오프닝 시네마틱 -> 새 게임
  void StartNewRun();                 // 새 게임 시작 (스테이지 1)
  void StartAtStage(int stage);       // 메인 메뉴에서 해당 스테이지로 바로 시작 (앞 스테이지를 건너뛴 만큼 기본 강화 지급)
  void StartBossFromShop();           // 메인 메뉴의 BOSS BATTLE: 코인 150으로 상점 -> 보스전
  void StartStage(bool retry);       // 현재 session.stage 시작. retry면 스냅샷을 갱신하지 않는다.
  void RestartStage();                // 일시정지 메뉴의 RESTART: 목숨 차감 없이 스테이지 시작 시점으로 되돌린다.
  void GoShop();                      // 스테이지 클리어 후 상점
  void LeaveShop();                   // 상점에서 다음 스테이지로
  void OnPlayerDefeated();            // 사망: 목숨 차감 후 재도전 또는 게임오버
  void GoGameOver();
  void GoVictory();
  void GoGallery();  // 개발용: 모든 도트 스프라이트 보기

 private:
  void ChangeMode(std::unique_ptr<GameMode> mode);  // 다음 프레임 시작 시 적용
  void GrantDebugCards();
  double SmoothFrameTime(double dt);  // 프레임 간격을 평균 내고 vsync 간격에 맞춘다 (끊김 방지)
  void RecordPerf(double dt, int steps, uint64_t renderStart, uint64_t presentStart);

  struct PerfStats {
    int frames = 0;
    double time = 0.0, maxDt = 0.0, renderMs = 0.0, presentMs = 0.0;
    int steps[3] = {0, 0, 0};  // 프레임당 로직 스텝 0 / 1 / 2 이상
    long long drawCalls = 0;
  };
  PerfStats perf_;

  Graphics gfx_;
  Input input_;
  Audio audio_;
  Session session_;
  GameContext ctx_;

  std::unique_ptr<GameMode> mode_;
  std::unique_ptr<GameMode> pending_;
  bool running_ = true;
  bool pendingValid_ = false;
  uint64_t lastCounter_ = 0;
  double accumulator_ = 0.0;
  static constexpr int kDtHistory = 4;
  double dtHistory_[kDtHistory] = {};
  int dtIndex_ = 0;
  int refreshHz_ = 0;
};

}  // namespace gengar

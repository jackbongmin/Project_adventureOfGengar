#include "core/Game.h"

#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "art/Art.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "core/Version.h"
#include "mode/GameOverMode.h"
#include "mode/IntroMode.h"
#include "mode/PlayMode.h"
#include "mode/ShopMode.h"
#include "mode/SpriteGalleryMode.h"
#include "mode/TitleMode.h"
#include "mode/VictoryMode.h"
#include "ui/Menu.h"

namespace gengar {

Game::Game() : ctx_{input_, audio_, session_} {}

Game::~Game() {
  mode_.reset();
  pending_.reset();
  audio_.Shutdown();
  gfx_.Shutdown();
  SDL_Quit();
}

bool Game::Init() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return false;
  }
  const std::string title = std::string("Adventure of Gengar ") + version::kLabel;
  if (!gfx_.Init(title.c_str(), cfg::kScreenW, cfg::kScreenH)) {
    SDL_Log("Graphics init failed: %s", SDL_GetError());
    return false;
  }
  art::Init(gfx_);  // 도트 스프라이트 텍스처 생성
  if (!audio_.Init()) SDL_Log("Audio init failed (sound disabled): %s", SDL_GetError());
  refreshHz_ = gfx_.RefreshRate();
  for (double& d : dtHistory_) d = static_cast<double>(cfg::kFixedStep);
  lastCounter_ = SDL_GetPerformanceCounter();
  GoTitle();
  return true;
}

void Game::ChangeMode(std::unique_ptr<GameMode> mode) {
  pending_ = std::move(mode);
  pendingValid_ = true;
}

void Game::Frame() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) running_ = false;
    input_.OnEvent(e);
  }

  const uint64_t now = SDL_GetPerformanceCounter();
  const double dt = static_cast<double>(now - lastCounter_) / static_cast<double>(SDL_GetPerformanceFrequency());
  lastCounter_ = now;
  accumulator_ += SmoothFrameTime(std::min(dt, 0.1)) * static_cast<double>(Debug().timeScale);

  int steps = 0;
  while (accumulator_ >= static_cast<double>(cfg::kFixedStep)) {
    if (pendingValid_) { mode_ = std::move(pending_); pendingValid_ = false; }
    if (mode_) mode_->Update(*this);
    input_.ClearPressed();
    accumulator_ -= static_cast<double>(cfg::kFixedStep);
    ++steps;
  }
  if (pendingValid_) { mode_ = std::move(pending_); pendingValid_ = false; }

  const uint64_t renderStart = SDL_GetPerformanceCounter();
  gfx_.Begin(Rgb(0, 0, 0));
  if (mode_) mode_->Render(gfx_);
  DrawVersion(gfx_);  // 어느 화면에서든 오른쪽 아래에 버전 (빌드 구분용)
  const uint64_t presentStart = SDL_GetPerformanceCounter();
  gfx_.End();
  if (Debug().perf) RecordPerf(dt, steps, renderStart, presentStart);
}

// 측정한 프레임 간격은 vsync 가 켜져 있어도 15ms, 18ms, 0.3ms, 33ms 처럼 들쭉날쭉하다. 그대로 누적하면
// 누적값이 스텝 경계(16.67ms)를 오락가락해서 로직이 0번 도는 프레임(멈춤)과 2번 도는 프레임(건너뜀)이 섞인다.
// 최근 4프레임의 평균을 쓰고, 그 평균이 vsync 간격의 정수배에 가까우면 정확히 그 값으로 맞춘다.
double Game::SmoothFrameTime(double dt) {
  dtHistory_[dtIndex_] = dt;
  dtIndex_ = (dtIndex_ + 1) % kDtHistory;
  double avg = 0.0;
  for (double d : dtHistory_) avg += d;
  avg /= kDtHistory;

  const double step = static_cast<double>(cfg::kFixedStep);
  double vsync = refreshHz_ > 0 ? 1.0 / static_cast<double>(refreshHz_) : step;
  if (std::abs(vsync - step) < 0.0002) vsync = step;  // 59.94Hz 같은 모니터도 60Hz 로 본다 (누적값이 스텝과 딱 맞도록)
  for (int k = 1; k <= 4; ++k) {
    if (std::abs(avg - vsync * k) < 0.002) return vsync * k;
  }
  return avg;
}

// --perf: 2초마다 프레임 간격, 렌더(CPU) 시간, 프레임당 로직 스텝 수 분포, 그리기 호출 수를 출력한다.
// 스텝이 0 이나 2 인 프레임은 화면이 한 번 멈췄다가 두 칸 건너뛰는 프레임이다 (= 눈에 보이는 끊김).
void Game::RecordPerf(double dt, int steps, uint64_t renderStart, uint64_t presentStart) {
  const double freq = static_cast<double>(SDL_GetPerformanceFrequency());
  const uint64_t end = SDL_GetPerformanceCounter();
  PerfStats& p = perf_;
  ++p.frames;
  p.time += dt;
  p.maxDt = std::max(p.maxDt, dt);
  p.renderMs += static_cast<double>(presentStart - renderStart) / freq * 1000.0;
  p.presentMs += static_cast<double>(end - presentStart) / freq * 1000.0;
  ++p.steps[std::min(steps, 2)];
  p.drawCalls += gfx_.DrawCalls();
  if (p.time < 2.0) return;
  const double n = static_cast<double>(p.frames);
  std::printf("[perf] %s %dHz fps=%.1f frame avg=%.2fms max=%.2fms | render=%.2fms present=%.2fms | steps 0:%d 1:%d 2+:%d | draws=%d\n",
              gfx_.RendererName(), gfx_.RefreshRate(), n / p.time, p.time / n * 1000.0, p.maxDt * 1000.0, p.renderMs / n, p.presentMs / n,
              p.steps[0], p.steps[1], p.steps[2], static_cast<int>(p.drawCalls / p.frames));
  std::fflush(stdout);
  p = PerfStats{};
}

void Game::Run() {
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg([](void* self) { static_cast<Game*>(self)->Frame(); }, this, 0, 1);
#else
  while (running_) Frame();
#endif
}

void Game::GoTitle() { ChangeMode(std::make_unique<TitleMode>()); }
void Game::GoIntro() { ChangeMode(std::make_unique<IntroMode>()); }

void Game::StartNewRun() {
  session_.Reset();
  session_.stage = Clamp(Debug().startStage, 0, cfg::kStageCount - 1);
  session_.coins = Debug().startCoins;
  if (Debug().startInShop) { GoShop(); return; }
  StartStage(false);
}

// 앞 스테이지를 건너뛴 만큼 상점에서 샀을 법한 강화를 미리 지급한다.
void Game::StartAtStage(int stage) {
  session_.Reset();
  session_.stage = Clamp(stage, 0, cfg::kStageCount - 1);
  auto give = [&](UpgradeId id, int lv) { session_.Level(id) = lv; };
  if (session_.stage >= 1) {
    give(UpgradeId::Power, 1); give(UpgradeId::Rate, 1); give(UpgradeId::MaxHp, 1); give(UpgradeId::Speed, 1);
  }
  StartStage(false);
}

// 보스전 연습: 업그레이드 없이 코인 150개를 들고 상점에서 시작한다. 상점을 나가면 바로 보스전이다.
void Game::StartBossFromShop() {
  session_.Reset();
  session_.stage = cfg::kBossStage - 1;  // 상점을 나가면 stage + 1 = 보스 스테이지
  session_.coins = 150;
  GoShop();
}

void Game::StartStage(bool retry) {
  if (!retry) session_.TakeSnapshot();
  DebugLog("[game] stage %d start (retry=%d) coins=%d lives=%d score=%d", session_.stage + 1, retry ? 1 : 0, session_.coins, session_.lives, session_.score);
  ChangeMode(std::make_unique<PlayMode>(*this, session_.stage));
}

void Game::RestartStage() {
  DebugLog("[game] stage %d restart from pause menu", session_.stage + 1);
  session_.RestoreSnapshot();
  StartStage(true);
}

void Game::GoShop() {
  DebugLog("[game] shop open coins=%d", session_.coins); ChangeMode(std::make_unique<ShopMode>(*this)); }

void Game::LeaveShop() {
  ++session_.stage;
  StartStage(false);
}

void Game::OnPlayerDefeated() {
  --session_.lives;
  DebugLog("[game] player defeated, lives left=%d", session_.lives);
  if (session_.lives <= 0) {
    GoGameOver();
  } else {
    session_.RestoreSnapshot();
    StartStage(true);
  }
}

void Game::GoGameOver() {
  DebugLog("[game] GAME OVER score=%d", session_.score); ChangeMode(std::make_unique<GameOverMode>()); }
void Game::GoGallery() { ChangeMode(std::make_unique<SpriteGalleryMode>()); }
void Game::GoVictory() {
  DebugLog("[game] VICTORY score=%d lives=%d", session_.score, session_.lives); ChangeMode(std::make_unique<VictoryMode>()); }

}  // namespace gengar

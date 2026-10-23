#include "core/Game.h"

#include <SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "art/Art.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "mode/GameOverMode.h"
#include "mode/IntroMode.h"
#include "mode/PlayMode.h"
#include "mode/ShopMode.h"
#include "mode/SpriteGalleryMode.h"
#include "mode/TitleMode.h"
#include "mode/VictoryMode.h"

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
  if (!gfx_.Init("Adventure of Gengar", cfg::kScreenW, cfg::kScreenH)) {
    SDL_Log("Graphics init failed: %s", SDL_GetError());
    return false;
  }
  art::Init(gfx_);  // 도트 스프라이트 텍스처 생성
  if (!audio_.Init()) SDL_Log("Audio init failed (sound disabled): %s", SDL_GetError());
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
  accumulator_ += std::min(dt, 0.1) * static_cast<double>(Debug().timeScale);

  while (accumulator_ >= static_cast<double>(cfg::kFixedStep)) {
    if (pendingValid_) { mode_ = std::move(pending_); pendingValid_ = false; }
    if (mode_) mode_->Update(*this);
    input_.ClearPressed();
    accumulator_ -= static_cast<double>(cfg::kFixedStep);
  }
  if (pendingValid_) { mode_ = std::move(pending_); pendingValid_ = false; }

  gfx_.Begin(Rgb(0, 0, 0));
  if (mode_) mode_->Render(gfx_);
  gfx_.End();
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

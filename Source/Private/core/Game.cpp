#include "core/Game.h"

#include <SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "art/Art.h"
#include "core/Config.h"
#include "mode/GameOverMode.h"
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
  accumulator_ += std::min(dt, 0.1);

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

void Game::StartNewRun() {
  session_.Reset();
  StartStage(false);
}

void Game::StartStage(bool retry) {
  if (!retry) session_.TakeSnapshot();
  ChangeMode(std::make_unique<PlayMode>(*this, session_.stage));
}

void Game::GoShop() { ChangeMode(std::make_unique<ShopMode>(*this)); }

void Game::LeaveShop() {
  ++session_.stage;
  StartStage(false);
}

void Game::OnPlayerDefeated() {
  --session_.lives;
  if (session_.lives <= 0) {
    GoGameOver();
  } else {
    session_.RestoreSnapshot();
    StartStage(true);
  }
}

void Game::GoGameOver() { ChangeMode(std::make_unique<GameOverMode>()); }
void Game::GoGallery() { ChangeMode(std::make_unique<SpriteGalleryMode>()); }
void Game::GoVictory() { ChangeMode(std::make_unique<VictoryMode>()); }

}  // namespace gengar

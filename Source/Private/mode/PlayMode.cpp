#include "mode/PlayMode.h"

#include "core/Config.h"
#include "core/Game.h"
#include "platform/Graphics.h"
#include "state/play/PlayPhases.h"
#include "ui/Hud.h"

namespace gengar {

PlayMode::PlayMode(Game& game, int stageIndex)
    : game_(game), world_(std::make_unique<World>(game.Context(), stageIndex)), phase_(*this) {
  phase_.Change(MakePlayingPhase());
  phase_.Apply();
}

void PlayMode::Update(Game&) { phase_.Update(); }

void PlayMode::Render(Graphics& gfx) {
  world_->Render(gfx);
  const float flash = world_->Fx().FlashAmount();
  if (flash > 0.f) {
    gfx.FillRect(0, 0, static_cast<float>(cfg::kScreenW), static_cast<float>(cfg::kScreenH),
                 Rgb(230, 200, 255, static_cast<int>(flash * 140.f)));
  }
  DrawHud(gfx, *world_, game_.Sess());
  phase_.Render(gfx);
}

}  // namespace gengar

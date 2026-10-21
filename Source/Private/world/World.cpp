#include "world/World.h"

#include <algorithm>

#include "actor/Player.h"
#include "actor/boss/Boss.h"
#include "actor/enemy/Bat.h"
#include "actor/enemy/Caster.h"
#include "actor/enemy/Hound.h"
#include "actor/enemy/Slime.h"
#include "actor/pickup/Pickups.h"
#include "core/DebugOptions.h"
#include "core/Random.h"
#include "core/Session.h"
#include "data/PlayerStats.h"
#include "platform/Graphics.h"
#include "world/LevelRenderer.h"

namespace gengar {

World::World(GameContext& ctx, int stageIndex)
    : ctx_(ctx), stage_(stageIndex), level_(BuildStage(stageIndex)) {
  player_ = Spawn<Player>(Vec2{2.f * cfg::kTile, 13.f * cfg::kTile}, stats::MaxHp(ctx_.session));
  if (Debug().startHp > 0) player_->SetHp(Debug().startHp);
  if (Debug().heartDemo) Spawn<Heart>(Vec2{3.f * cfg::kTile + 16.f, 14.f * cfg::kTile + 16.f});
  for (const SpawnPoint& sp : level_.Spawns()) {
    switch (sp.type) {
      case SpawnType::Slime: Spawn<Slime>(sp.pos, stage_); break;
      case SpawnType::Bat: Spawn<Bat>(sp.pos, stage_, sp.range); break;
      case SpawnType::Hound: Spawn<Hound>(sp.pos, stage_); break;
      case SpawnType::Caster: Spawn<Caster>(sp.pos, stage_); break;
      case SpawnType::Coin: Spawn<Coin>(sp.pos); break;
      case SpawnType::Heart: Spawn<Heart>(sp.pos); break;
      case SpawnType::Boss: {
        const int hp = Debug().bossHp > 0 ? Debug().bossHp : cfg::kBossHp;
        boss_ = Spawn<Boss>(sp.pos, hp);
        if (Debug().bossPhase > 0) boss_->SetHealthRatio(Debug().bossPhase == 1 ? 0.6f : 0.25f);
        break;
      }
    }
  }
  ShowBanner(level_.Name(), 150);
}

World::~World() = default;

int World::CountAlive(ActorKind kind) const {
  return static_cast<int>(std::count_if(actors_.begin(), actors_.end(),
                                        [kind](const std::unique_ptr<Actor>& a) { return a->Kind() == kind && !a->IsDead(); }));
}

void World::Update() {
  ++frame_;
  level_.UpdateCrumbles(*this);
  for (size_t i = 0; i < actors_.size(); ++i) {
    Actor* a = actors_[i].get();
    if (!a->IsDead()) a->Update(*this);
  }
  actors_.erase(std::remove_if(actors_.begin(), actors_.end(),
                               [this](const std::unique_ptr<Actor>& a) {
                                 if (!a->IsDead()) return false;
                                 if (a.get() == static_cast<Actor*>(boss_)) boss_ = nullptr;
                                 return true;
                               }),
                actors_.end());
  fx_.Update();
  camera_.Update(player_->Bounds(), player_->Facing(), level_.PixelWidth());
  if (bannerTimer_ > 0) --bannerTimer_;
}

void World::UpdateEffects() { fx_.Update(); }

void World::Render(Graphics& gfx) const {
  const float s = fx_.ShakeAmount();
  const float sx = s > 0.f ? RandRange(-s, s) : 0.f, sy = s > 0.f ? RandRange(-s, s) : 0.f;

  gfx.SetOffset(sx, sy);
  RenderLevelBackground(gfx, level_, camera_.x, frame_);

  gfx.SetOffset(camera_.x + sx, sy);
  RenderLevelTiles(gfx, level_, camera_.x, frame_);

  for (int k = 0; k < kActorKindCount; ++k) {
    const ActorKind kind = static_cast<ActorKind>(k);
    for (const auto& a : actors_) {
      if (a->Kind() == kind && !a->IsDead()) a->Render(gfx, *this);
    }
  }
  fx_.Render(gfx);
  gfx.SetOffset(0.f, 0.f);
}

void World::AddCoins(int n) { ctx_.session.coins += n; }
void World::AddScore(int n) { ctx_.session.score += n; }
void World::AddGauge(float n) {
  if (ultActive_) return;
  Session& s = ctx_.session;
  s.gauge = std::min(100.f, s.gauge + n * stats::GaugeMultiplier(s));
}

}  // namespace gengar

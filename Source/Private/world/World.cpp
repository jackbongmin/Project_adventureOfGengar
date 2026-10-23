#include "world/World.h"

#include <algorithm>
#include <cmath>

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
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "world/LevelRenderer.h"

namespace gengar {

World::World(GameContext& ctx, int stageIndex)
    : ctx_(ctx), stage_(stageIndex), level_(BuildStage(stageIndex)) {
  actors_.reserve(256);
  player_ = Spawn<Player>(Vec2{2.f * cfg::kTile, 13.f * cfg::kTile}, stats::MaxHp(ctx_.session));
  if (Debug().startHp > 0) player_->SetHp(Debug().startHp);
  if (Debug().heightDemo) {  // 스테이지 1의 낮은 발판(12행) 위에 플레이어, 바로 아래 바닥에 추적 중인 늑대
    constexpr float T = static_cast<float>(cfg::kTile);
    player_->pos = {13.f * T, 12.f * T - 31.f};
    Spawn<Hound>(Vec2{14.f * T, 15.f * T - 24.f}, stage_)->ForceAlert();
  }
  if (Debug().stompDemo > 0) {  // 높은 곳에서 떨어져 가만히 선 늑대를 밟는다. 튕긴 뒤 다시 떨어져 한 번 더 밟는다.
    constexpr float T = static_cast<float>(cfg::kTile);
    player_->pos = {5.f * T + 6.f, 5.f * T + static_cast<float>(Debug().stompDemo - 1)};
    Spawn<Hound>(Vec2{5.f * T, 15.f * T - 24.f}, stage_)->SetController(nullptr);
  }
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
        const int hp = Debug().bossHp > 0 ? Debug().bossHp : stats::BossMaxHp(ctx_.session);
        boss_ = Spawn<Boss>(sp.pos, hp);
        if (Debug().bossPhase > 0) boss_->SetHealthRatio(Debug().bossPhase == 1 ? 0.6f : 0.25f);
        break;
      }
    }
  }
  ShowBanner(level_.Name(), 150);
}

World::~World() {
  DebugLog("[world] pooled actors: created %d, reused %d", pooledCreated_, pooledReused_);
}

int World::CountAlive(ActorKind kind) const {
  return static_cast<int>(std::count_if(actors_.begin(), actors_.end(),
                                        [kind](const std::unique_ptr<Actor>& a) { return a->Kind() == kind && !a->IsDead(); }));
}

void World::Update() {
  if (hitStop_ > 0) {  // 히트스톱 중에는 아무것도 움직이지 않는다. 그 사이 눌린 점프/필살기는 플레이어가 기억해 둔다.
    --hitStop_;
    player_->LatchInput(*this);
    return;
  }
  ++frame_;
  level_.UpdateCrumbles(*this);
  for (size_t i = 0; i < actors_.size(); ++i) {
    Actor* a = actors_[i].get();
    if (!a->IsDead()) a->Update(*this);
  }
  // 죽은 액터 정리: 살아 있는 것은 앞으로 당기고, 풀링 대상은 풀로 돌려보내고, 나머지는 해제한다.
  size_t keep = 0;
  for (size_t i = 0; i < actors_.size(); ++i) {
    std::unique_ptr<Actor>& a = actors_[i];
    if (!a->IsDead()) {
      if (keep != i) actors_[keep] = std::move(a);
      ++keep;
      continue;
    }
    if (a.get() == static_cast<Actor*>(boss_)) boss_ = nullptr;
    if (a->pooled_) {
      const Actor& dead = *a;
      pools_[std::type_index(typeid(dead))].push_back(std::move(a));
    }
  }
  actors_.resize(keep);  // 뒤에 남은 죽은 액터(풀링 대상이 아닌 것)가 여기서 해제된다
  fx_.Update();
  camera_.Update(player_->Bounds(), player_->Facing(), level_.PixelWidth());
  if (bannerTimer_ > 0) --bannerTimer_;
  if (comboTimer_ > 0 && --comboTimer_ == 0) combo_ = 0;
  if (comboPop_ > 0) --comboPop_;
}

void World::UpdateEffects() { fx_.Update(); }

void World::Render(Graphics& gfx) const {
  const float s = fx_.ShakeAmount();
  // 카메라와 흔들림은 정수 픽셀로 맞춘다. 소수점 오프셋이면 스프라이트(floor)와 사각형(round)이 서로 다르게
  // 반올림돼서, 카메라가 따라가는 동안 캐릭터와 타일 사이가 1px 씩 떨린다.
  const float sx = s > 0.f ? std::round(RandRange(-s, s)) : 0.f, sy = s > 0.f ? std::round(RandRange(-s, s)) : 0.f;
  const float camX = std::round(camera_.x);

  gfx.SetOffset(sx, sy);
  RenderLevelBackground(gfx, level_, camX, frame_);

  gfx.SetOffset(camX + sx, sy);
  RenderLevelTiles(gfx, level_, camX, frame_);

  for (int k = 0; k < kActorKindCount; ++k) {
    const ActorKind kind = static_cast<ActorKind>(k);
    for (const auto& a : actors_) {
      if (a->Kind() == kind && !a->IsDead()) a->Render(gfx, *this);
    }
  }
  fx_.Render(gfx);
  gfx.SetOffset(0.f, 0.f);
}

int World::RegisterKill(Vec2 at) {
  ++combo_;
  comboTimer_ = kComboFrames;
  comboPop_ = 12;
  if (combo_ >= 2) Snd().Tone(520.f * std::pow(1.06f, static_cast<float>(std::min(combo_, 24))), 0.f, 0.07f, Wave::Square, 0.04f);
  if (combo_ % 5 == 0) {
    AddCoins(5);
    fx_.Pop({at.x, at.y - 30.f}, std::to_string(combo_) + " COMBO +5", Hex(0xffb347));
    Snd().Play(Sfx::Coin);
  }
  return ComboMultiplier();
}

void World::AddCoins(int n) { ctx_.session.coins += n; }
void World::AddScore(int n) { ctx_.session.score += n; }
void World::AddGauge(float n) {
  if (ultActive_) return;
  Session& s = ctx_.session;
  s.gauge = std::min(100.f, s.gauge + n * stats::GaugeMultiplier(s));
}

}  // namespace gengar

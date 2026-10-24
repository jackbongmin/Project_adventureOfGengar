#include "world/World.h"

#include <algorithm>
#include <cmath>

#include "actor/Player.h"
#include "actor/boss/Boss.h"
#include "actor/enemy/Bat.h"
#include "actor/enemy/Caster.h"
#include "actor/enemy/Hopper.h"
#include "actor/enemy/Hound.h"
#include "actor/enemy/Slime.h"
#include "actor/pickup/Pickups.h"
#include "actor/projectile/PlayerSkills.h"
#include "core/DebugOptions.h"
#include "core/Random.h"
#include "core/Session.h"
#include "data/Cards.h"
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
  for (const SpawnPoint& sp : level_.Spawns()) SpawnFrom(sp, 0);
  arenaDone_.assign(level_.Arenas().size(), false);
  ShowBanner(level_.Name(), 150);

  // 스테이지를 시작할 때 적용되는 카드
  Session& s = ctx_.session;
  if (s.Has(CardId::Shield)) player_->GiveShield();
  if (s.Has(CardId::HeadStart)) s.gauge = std::max(s.gauge, 30.f);
  for (int i = 0; i < s.Card(CardId::Familiar); ++i) Spawn<Familiar>(player_->Bounds().Center(), i);
  rerollsLeft_ = stats::Rerolls(s);
  pendingPicks_ = Debug().startPicks;
}

void World::SpawnFrom(const SpawnPoint& sp, int arenaTag) {
  Enemy* e = nullptr;
  switch (sp.type) {
    case SpawnType::Slime: e = Spawn<Slime>(sp.pos, stage_); break;
    case SpawnType::Bat: e = Spawn<Bat>(sp.pos, stage_, sp.range); break;
    case SpawnType::Hound: {
      Hound* h = Spawn<Hound>(sp.pos, stage_);
      if (arenaTag > 0) h->ForceAlert();  // 결투장 늑대는 나오자마자 달려든다
      e = h;
      break;
    }
    case SpawnType::Caster: e = Spawn<Caster>(sp.pos, stage_); break;
    case SpawnType::Hopper: e = Spawn<Hopper>(sp.pos, stage_); break;
    case SpawnType::Coin: Spawn<Coin>(sp.pos); break;
    case SpawnType::Heart: Spawn<Heart>(sp.pos); break;
    case SpawnType::Chest: Spawn<Chest>(sp.pos); break;
    case SpawnType::Boss: {
      const int hp = Debug().bossHp > 0 ? Debug().bossHp : stats::BossMaxHp(ctx_.session);
      boss_ = Spawn<Boss>(sp.pos, hp);
      if (Debug().bossPhase > 0) boss_->SetHealthRatio(Debug().bossPhase == 1 ? 0.6f : 0.25f);
      break;
    }
  }
  if (!e) return;
  if (sp.elite) e->MakeElite();
  e->arenaTag = arenaTag;
}

// 결투장 양 끝 열의 빈칸을 마법 벽으로 채우거나 다시 비운다
void World::SetArenaWalls(const ArenaDef& a, bool on) {
  for (int c : {a.c0, a.c1}) {
    for (int r = 0; r < cfg::kGroundRow; ++r) {
      const Tile t = level_.At(c, r);
      if (on && t == Tile::Empty) level_.Set(c, r, Tile::Barrier);
      else if (!on && t == Tile::Barrier) level_.Set(c, r, Tile::Empty);
    }
  }
}

// 매복 결투장: 들어오면 벽이 닫히고 웨이브가 차례로 나온다. 다 잡으면 벽이 열리고 보물상자.
void World::UpdateArenas() {
  constexpr float T = static_cast<float>(cfg::kTile);
  const std::vector<ArenaDef>& arenas = level_.Arenas();
  if (arena_ < 0) {
    const float px = player_->Bounds().Center().x;
    for (size_t i = 0; i < arenas.size(); ++i) {
      if (arenaDone_[i]) continue;
      const ArenaDef& a = arenas[i];
      if (px < static_cast<float>(a.c0 + 3) * T || px > static_cast<float>(a.c1 - 2) * T) continue;
      arena_ = static_cast<int>(i);
      arenaWave_ = 0;
      arenaTimer_ = 50;
      SetArenaWalls(a, true);
      ShowBanner("AMBUSH!", 70);
      fx_.Shake(8.f);
      Snd().Play(Sfx::Alert);
      Snd().Play(Sfx::BossSummon);
      DebugLog("[arena] %d start (cols %d-%d, %d waves)", arena_, a.c0, a.c1, static_cast<int>(a.waves.size()));
      return;
    }
    return;
  }
  const ArenaDef& a = arenas[static_cast<size_t>(arena_)];
  if (arenaTimer_ > 0) {
    if (--arenaTimer_ > 0) return;
    // 웨이브 소환: 나올 자리마다 보랏빛(정예는 금빛) 소용돌이
    for (const SpawnPoint& sp : a.waves[static_cast<size_t>(arenaWave_)]) {
      SpawnFrom(sp, arena_ + 1);
      const Vec2 at{sp.pos.x + 12.f, sp.pos.y + 12.f};
      fx_.Burst(at, sp.elite ? 30 : 16, sp.elite ? Hex(0xffc93c) : Hex(0xb36bff), 4.f, 26.f);
      fx_.AddRing(at, 30.f);
    }
    Snd().Play(Sfx::BossBlink);
    arenaClock_ = arenaWave_ + 1 < static_cast<int>(a.waves.size()) ? ArenaClockMax() : 0;
    DebugLog("[arena] %d wave %d/%d", arena_, arenaWave_ + 1, static_cast<int>(a.waves.size()));
    return;
  }
  int alive = 0;
  ForEach(ActorKind::Enemy, [&](Actor& e) { if (static_cast<Enemy&>(e).arenaTag == arena_ + 1) ++alive; });
  const bool last = arenaWave_ + 1 >= static_cast<int>(a.waves.size());
  if (!last && alive > 0 && arenaClock_ > 0) {  // 제한 시간: 마지막 5초는 1초마다 째깍, 끝나면 증원
    --arenaClock_;
    if (arenaClock_ > 0 && arenaClock_ <= 300 && arenaClock_ % 60 == 0) Snd().Tone(880.f, 0.f, 0.06f, Wave::Square, 0.05f);
    if (arenaClock_ == 0) {
      ++arenaWave_;
      arenaTimer_ = 45;
      ShowBanner("REINFORCEMENTS!", 70);
      fx_.Shake(6.f);
      Snd().Play(Sfx::Alert);
      DebugLog("[arena] %d time up -> reinforcements", arena_);
      return;
    }
  }
  if (alive > 0) {
    if (frame_ % 300 == 0) {  // 오래 안 끝나면 어디에 남았는지 남긴다 (검증용)
      ForEach(ActorKind::Enemy, [&](Actor& e) {
        if (static_cast<Enemy&>(e).arenaTag == arena_ + 1) DebugLog("[arena] left: x=%d y=%d hp=%d", static_cast<int>(e.pos.x), static_cast<int>(e.pos.y), static_cast<Enemy&>(e).Hp());
      });
    }
    return;
  }
  if (!last && arenaClock_ > 0) {  // 빨리 정리했으면 남은 3초마다 코인 1
    const int bonus = arenaClock_ / 180;
    if (bonus > 0) {
      AddCoins(bonus);
      const Vec2 c = player_->Bounds().Center();
      fx_.Pop({c.x, c.y - 44.f}, "FAST +" + std::to_string(bonus), Hex(0xffe27a));
      Snd().Play(Sfx::Coin);
    }
  }
  ++arenaWave_;
  if (arenaWave_ < static_cast<int>(a.waves.size())) {
    arenaTimer_ = 60;
    ShowBanner("WAVE " + std::to_string(arenaWave_ + 1) + "/" + std::to_string(a.waves.size()), 60);
    return;
  }
  // 끝: 벽을 열고 가운데에 보물상자를 떨어뜨린다
  SetArenaWalls(a, false);
  arenaDone_[static_cast<size_t>(arena_)] = true;
  const float cx = static_cast<float>(a.c0 + a.c1 + 1) * 0.5f * T;
  Spawn<Chest>(Vec2{cx, 3.f * T});
  ShowBanner("AMBUSH CLEAR!", 90);
  fx_.Shake(6.f);
  Snd().Play(Sfx::StageClear);
  DebugLog("[arena] %d clear", arena_);
  arena_ = -1;
}

int World::ArenaWaves() const { return arena_ < 0 ? 0 : static_cast<int>(level_.Arenas()[static_cast<size_t>(arena_)].waves.size()); }
// 웨이브 제한 시간: 스테이지 1은 20초, 2는 16초
int World::ArenaClockMax() const { return stage_ == 0 ? 20 * 60 : 16 * 60; }

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
  if (levelUpFlash_ > 0) --levelUpFlash_;
  const bool frozen = timeStop_ > 0;  // 시간 정지: 플레이어 쪽(플레이어, 내 탄, 분신, 아이템)만 움직인다
  if (!frozen) level_.UpdateCrumbles(*this);
  if (!frozen) UpdateArenas();
  for (size_t i = 0; i < actors_.size(); ++i) {
    Actor* a = actors_[i].get();
    if (a->IsDead()) continue;
    const ActorKind k = a->Kind();
    if (frozen && k != ActorKind::Player && k != ActorKind::PlayerShot && k != ActorKind::Pickup) continue;
    a->Update(*this);
  }
  if (timeStop_ > 0 && --timeStop_ == 0) EndTimeStop();
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
  if (arena_ >= 0) {  // 결투장 안에서는 카메라가 결투장 화면에 묶인다
    const ArenaDef& a = level_.Arenas()[static_cast<size_t>(arena_)];
    const float T = static_cast<float>(cfg::kTile);
    camera_.Update(player_->Bounds(), player_->Facing(), level_.PixelWidth(), static_cast<float>(a.c0) * T,
                   static_cast<float>(a.c1 + 1) * T - static_cast<float>(cfg::kScreenW));
  } else {
    camera_.Update(player_->Bounds(), player_->Facing(), level_.PixelWidth());
  }
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

  // 결투장 소환 예고: 다음 웨이브가 나올 자리 바닥에 도는 마법진 (정예 자리는 금빛)
  if (arena_ >= 0 && arenaTimer_ > 0) {
    const ArenaDef& a = level_.Arenas()[static_cast<size_t>(arena_)];
    const float k = 1.f - static_cast<float>(arenaTimer_) / 60.f;
    for (const SpawnPoint& sp : a.waves[static_cast<size_t>(arenaWave_)]) {
      const Vec2 at{sp.pos.x + 12.f, sp.pos.y + 12.f};
      const Color c = sp.elite ? Rgb(255, 201, 60, 200) : Rgb(200, 110, 255, 200);
      gfx.Glow(at.x, at.y, 20.f + 20.f * k, sp.elite ? Rgb(255, 180, 60, 90) : Rgb(180, 90, 255, 90));
      gfx.PixelRing(at.x, at.y, 8.f + 18.f * k, 2.f, c, 2.f);
      if ((frame_ / 4) % 2 == 0) gfx.PixelRing(at.x, at.y, 26.f - 10.f * k, 2.f, c, 2.f);
    }
  }
  // 정예 적: 뒤에 금빛 기운 (몸보다 먼저 그린다)
  for (const auto& a : actors_) {
    if (a->IsDead() || a->Kind() != ActorKind::Enemy || !static_cast<const Enemy&>(*a).Elite()) continue;
    const Rect b = a->Bounds();
    const float pulse = std::sin(static_cast<float>(frame_) * 0.12f);
    gfx.Glow(b.Center().x, b.Center().y, b.w * 1.3f + 8.f + pulse * 4.f, Rgb(255, 170, 60, 110));
    gfx.PixelRing(b.Center().x, b.Center().y, b.w * 0.75f + 6.f + pulse * 2.f, 2.f, Rgb(255, 214, 90, 160), 2.f);
  }
  for (int k = 0; k < kActorKindCount; ++k) {
    const ActorKind kind = static_cast<ActorKind>(k);
    for (const auto& a : actors_) {
      if (a->Kind() == kind && !a->IsDead()) a->Render(gfx, *this);
    }
  }
  // 독·냉기 표식: 머리 오른쪽 위에 작은 초록 방울 / 하늘색 눈송이
  for (const auto& a : actors_) {
    if (a->IsDead()) continue;
    bool poisoned = false, chilled = false;
    if (a->Kind() == ActorKind::Enemy) {
      poisoned = static_cast<const Enemy&>(*a).Poisoned();
      chilled = static_cast<const Enemy&>(*a).Chilled();
    } else if (a->Kind() == ActorKind::Boss) {
      poisoned = static_cast<const Boss&>(*a).Poisoned();
    }
    const Rect b = a->Bounds();
    const float x = std::floor(b.Right() / 2.f) * 2.f, y = b.y - 6.f;
    if (poisoned) {
      gfx.FillRect(x - 2.f, y - 6.f, 4.f, 2.f, Hex(0x6fe08a));
      gfx.FillRect(x - 4.f, y - 4.f, 8.f, 6.f, Hex(0x3a9a4a));
      gfx.FillRect(x - 2.f, y - 2.f, 2.f, 2.f, Hex(0xd8ffe0));
    }
    if (chilled) {
      const float cx = x - (poisoned ? 12.f : 0.f);
      gfx.FillRect(cx - 4.f, y - 2.f, 8.f, 2.f, Hex(0x9ff0ff));
      gfx.FillRect(cx - 1.f, y - 5.f, 2.f, 8.f, Hex(0x9ff0ff));
      gfx.FillRect(cx - 1.f, y - 2.f, 2.f, 2.f, Hex(0xffffff));
    }
  }
  // 저주 표식: 머리 위에 작은 보라 눈
  for (const auto& a : actors_) {
    if (a->IsDead()) continue;
    bool cursed = false;
    if (a->Kind() == ActorKind::Enemy) cursed = static_cast<const Enemy&>(*a).Cursed();
    else if (a->Kind() == ActorKind::Boss) cursed = static_cast<const Boss&>(*a).Cursed();
    if (!cursed) continue;
    const Rect b = a->Bounds();
    const float x = std::floor(b.Center().x / 2.f) * 2.f, y = b.y - 12.f + std::floor(std::sin(static_cast<float>(frame_) * 0.15f) * 1.5f) * 2.f;
    gfx.FillRect(x - 6.f, y, 12.f, 6.f, Hex(0x2a0a40));
    gfx.FillRect(x - 4.f, y + 2.f, 8.f, 2.f, Hex(0xc58bff));
    gfx.FillRect(x - 1.f, y + 1.f, 2.f, 4.f, Hex(0xff5ca8));
  }
  // 정예 적 체력 바와 스킬 예고 "!"
  for (const auto& a : actors_) {
    if (a->IsDead() || a->Kind() != ActorKind::Enemy) continue;
    const Enemy& e = static_cast<const Enemy&>(*a);
    if (!e.Elite()) continue;
    const Rect b = e.Bounds();
    if (e.Warn() > 0 && (e.Warn() / 3) % 2 == 0) gfx.Text("!", b.Center().x, b.y - 52.f, 4, Hex(0xff4a5a), Align::Center);
    const float w = 36.f, x = std::floor(b.Center().x - w * 0.5f), y = b.y - 22.f;
    gfx.FillRect(x - 2.f, y - 2.f, w + 4.f, 8.f, Hex(0x1a0b2e));
    gfx.FillRect(x, y, std::floor(w * static_cast<float>(e.Hp()) / static_cast<float>(e.MaxHp()) / 2.f) * 2.f, 4.f, Hex(0xffc93c));
  }
  fx_.Render(gfx);
  gfx.SetOffset(0.f, 0.f);
  if (arena_ >= 0) {  // 결투장: 화면 가장자리가 붉게 맥박친다. 제한 시간 5초 안쪽이면 더 진하고 빠르게
    const bool hurry = arenaClock_ > 0 && arenaClock_ <= 300;
    const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(frame_) * (hurry ? 0.3f : 0.08f));
    const int a0 = static_cast<int>((hurry ? 110.f : 55.f) * pulse);
    const float W = static_cast<float>(cfg::kScreenW), H = static_cast<float>(cfg::kScreenH);
    for (int i = 0; i < 4; ++i) {
      const float d = static_cast<float>(i) * 6.f;
      const Color c = Rgb(255, 40, 70, a0 / (i + 1));
      gfx.FillRect(0, d, W, 6, c);
      gfx.FillRect(0, H - d - 6.f, W, 6, c);
      gfx.FillRect(d, 0, 6, H, c);
      gfx.FillRect(W - d - 6.f, 0, 6, H, c);
    }
  }
  if (timeStop_ > 0) {  // 나이트메어: 화면을 보랏빛 흑백처럼 덮고, 끝나기 직전에 깜빡인다
    const int a = timeStop_ < 30 && (timeStop_ / 3) % 2 ? 40 : 90;
    gfx.FillRect(0, 0, static_cast<float>(cfg::kScreenW), static_cast<float>(cfg::kScreenH), Rgb(40, 0, 60, a));
    for (float y = 0; y < static_cast<float>(cfg::kScreenH); y += 4.f) gfx.FillRect(0, y, static_cast<float>(cfg::kScreenW), 2.f, Rgb(0, 0, 0, 30));
  }
}

int World::RegisterKill(Vec2 at) {
  ++combo_;
  comboTimer_ = ComboFrames();
  comboPop_ = 12;
  if (combo_ >= 2) Snd().Tone(520.f * std::pow(1.06f, static_cast<float>(std::min(combo_, 24))), 0.f, 0.07f, Wave::Square, 0.04f);
  if (combo_ % 5 == 0) {
    AddCoins(5);
    fx_.Pop({at.x, at.y - 30.f}, std::to_string(combo_) + " COMBO +5", Hex(0xffb347));
    Snd().Play(Sfx::Coin);
  }
  return ComboMultiplier();
}

int World::ComboFrames() const { return stats::ComboFrames(ctx_.session); }

void World::AddCoins(int n) {
  Session& s = ctx_.session;
  s.coinFrac += static_cast<float>(n) * stats::CoinMultiplier(s);  // 탐욕 카드: 소수점은 모아 두었다가 1이 되면 지급
  const int whole = static_cast<int>(s.coinFrac);
  s.coins += whole;
  s.coinFrac -= static_cast<float>(whole);
}

void World::AddXp(int n) {
  if (stage_ == cfg::kBossStage) return;  // 보스전은 체력 단계 전환으로 카드를 준다
  Session& s = ctx_.session;
  xpFrac_ += static_cast<float>(n) * stats::XpMultiplier(s);
  const int whole = static_cast<int>(xpFrac_);
  xpFrac_ -= static_cast<float>(whole);
  s.prog.xp += whole;
  while (s.prog.xp >= XpToNext(s.prog.level)) {
    s.prog.xp -= XpToNext(s.prog.level);
    ++s.prog.level;
    ++pendingPicks_;
    levelUpFlash_ = 40;
    DebugLog("[xp] LEVEL UP -> %d", s.prog.level);
    const Vec2 c = player_->Bounds().Center();
    fx_.Pop({c.x, c.y - 40.f}, "LEVEL UP!", Hex(0x7ae0ff));
    fx_.Burst(c, 24, Hex(0x7ae0ff), 4.f, 26.f);
    Snd().Tone(523.f, 1046.f, 0.18f, Wave::Square, 0.06f);
  }
}

void World::OnEnemyKilled() {
  ++kills_;
  if (ctx_.session.Has(CardId::SoulDrain) && kills_ % 12 == 0 && player_->Hp() < player_->MaxHp()) {
    player_->Heal(1);
    const Vec2 c = player_->Bounds().Center();
    fx_.Pop({c.x, c.y - 30.f}, "+HP", Hex(0xff7aa8));
    Snd().Play(Sfx::Heart);
  }
}

// 나이트메어가 끝나는 순간: 화면 안 모든 적에게 80, 보스에게 300 (필살기 위력 배율 적용)
void World::EndTimeStop() {
  const Session& s = ctx_.session;
  const float mul = stats::UltMultiplier(s);
  DamageInfo info{80.f * mul, 1, false, true};
  info.fromPlayer = true;
  const Rect view{camera_.x - 40.f, 0.f, static_cast<float>(cfg::kScreenW) + 80.f, static_cast<float>(cfg::kScreenH)};
  ForEach(ActorKind::Enemy, [&](Actor& a) {
    if (a.Bounds().Overlaps(view)) static_cast<Enemy&>(a).TakeDamage(*this, info);
  });
  if (boss_ && !boss_->dying) { info.amount = 300.f * mul; boss_->TakeDamage(*this, info); }
  fx_.Flash();
  fx_.Shake(14.f);
  fx_.AddRing(player_->Bounds().Center());
  Snd().Play(Sfx::Ultimate);
}
void World::AddScore(int n) { ctx_.session.score += n; }
void World::AddGauge(float n) {
  if (ultActive_) return;
  Session& s = ctx_.session;
  s.gauge = std::min(100.f, s.gauge + n * stats::GaugeMultiplier(s));
}

}  // namespace gengar

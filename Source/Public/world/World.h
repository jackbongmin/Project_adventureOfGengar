#pragma once
// 한 스테이지의 모든 것을 담는 컨테이너: 레벨, 액터 목록, 카메라, 이펙트.
// 액터는 Update(World&)로 이 객체를 통해 다른 액터/레벨/사운드에 접근한다.
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "actor/Actor.h"
#include "core/GameContext.h"
#include "world/Camera.h"
#include "world/Effects.h"
#include "world/Level.h"

namespace gengar {

class Player;
class Boss;
class Audio;
class Input;
struct Session;
class Graphics;

class World {
 public:
  World(GameContext& ctx, int stageIndex);
  ~World();

  void Update();         // 전체 갱신 (액터, 발판, 이펙트, 카메라)
  void UpdateEffects();  // 이펙트만 갱신 (사망/클리어 연출 중)
  void Render(Graphics& gfx) const;

  // ---- 액터 관리 ----
  template <class T, class... Args>
  T* Spawn(Args&&... args) {
    auto owned = std::make_unique<T>(std::forward<Args>(args)...);
    T* raw = owned.get();
    actors_.push_back(std::move(owned));
    return raw;
  }
  // 스폰이 일어나도 안전하도록 인덱스 기반으로 순회한다.
  template <class F>
  void ForEach(ActorKind kind, F&& fn) {
    for (size_t i = 0; i < actors_.size(); ++i) {
      Actor* a = actors_[i].get();
      if (a->Kind() == kind && !a->IsDead()) fn(*a);
    }
  }
  int CountAlive(ActorKind kind) const;

  // ---- 접근자 ----
  Player& GetPlayer() { return *player_; }
  const Player& GetPlayer() const { return *player_; }
  Boss* GetBoss() { return boss_; }
  const Boss* GetBoss() const { return boss_; }
  Level& GetLevel() { return level_; }
  const Level& GetLevel() const { return level_; }
  Effects& Fx() { return fx_; }
  const Effects& Fx() const { return fx_; }
  Camera& Cam() { return camera_; }
  const Camera& Cam() const { return camera_; }
  Audio& Snd() { return ctx_.audio; }
  Input& In() { return ctx_.input; }
  Session& Sess() { return ctx_.session; }
  const Session& Sess() const { return ctx_.session; }
  int Frame() const { return frame_; }
  int StageIndex() const { return stage_; }

  // ---- 게임 규칙 ----
  void AddCoins(int n);
  void AddScore(int n);
  void AddGauge(float n);  // 필살기 게이지 (필살기 발동 중에는 충전되지 않음)
  void SetUltActive(bool on) { ultActive_ = on; }

  void ShowBanner(const std::string& text, int frames) { banner_ = text; bannerTimer_ = frames; }
  const std::string& Banner() const { return banner_; }
  int BannerTimer() const { return bannerTimer_; }

  void NotifyStageCleared() { stageCleared_ = true; }
  bool StageCleared() const { return stageCleared_; }
  void NotifyBossDefeated() { bossDefeated_ = true; }
  bool BossDefeated() const { return bossDefeated_; }
  void NotifyHatCollected() { hatCollected_ = true; }  // 보물 모자를 되찾음 -> 엔딩
  bool HatCollected() const { return hatCollected_; }

 private:
  GameContext& ctx_;
  int stage_;
  Level level_;
  Camera camera_;
  Effects fx_;
  std::vector<std::unique_ptr<Actor>> actors_;
  Player* player_ = nullptr;
  Boss* boss_ = nullptr;
  int frame_ = 0;
  bool ultActive_ = false;
  bool stageCleared_ = false;
  bool bossDefeated_ = false;
  bool hatCollected_ = false;
  std::string banner_;
  int bannerTimer_ = 0;
};

}  // namespace gengar

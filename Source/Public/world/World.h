#pragma once
// 한 스테이지의 모든 것을 담는 컨테이너: 레벨, 액터 목록, 카메라, 이펙트.
// 액터는 Update(World&)로 이 객체를 통해 다른 액터/레벨/사운드에 접근한다.
#include <algorithm>
#include <memory>
#include <new>
#include <string>
#include <typeindex>
#include <unordered_map>
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
  // 오브젝트 풀링: 투사체처럼 자주 생겼다 사라지는 액터용.
  // 죽은 객체는 지우지 않고 타입별 풀에 돌려놓았다가, 같은 메모리에 생성자를 다시 호출해 재사용한다.
  // (생성자가 곧 초기화 코드라서 따로 Reset 함수를 만들 필요가 없다. 이 게임은 예외를 쓰지 않는다.)
  template <class T, class... Args>
  T* SpawnPooled(Args&&... args) {
    std::vector<std::unique_ptr<Actor>>& pool = pools_[std::type_index(typeid(T))];
    std::unique_ptr<Actor> owned;
    if (pool.empty()) {
      owned = std::make_unique<T>(std::forward<Args>(args)...);
      ++pooledCreated_;
    } else {
      owned = std::move(pool.back());
      pool.pop_back();
      T* old = static_cast<T*>(owned.get());
      std::destroy_at(old);
      ::new (static_cast<void*>(old)) T(std::forward<Args>(args)...);
      ++pooledReused_;
    }
    owned->pooled_ = true;
    T* raw = static_cast<T*>(owned.get());
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

  // ---- 로그라이크: 경험치와 카드 ----
  void AddXp(int n);              // 레벨이 오를 때마다 카드 고르기가 하나씩 쌓인다 (보스전에서는 무시)
  void QueueCardPick() { ++pendingPicks_; }  // 보스 체력 단계 전환 보상
  int PendingPicks() const { return pendingPicks_; }
  void ConsumePick() { if (pendingPicks_ > 0) --pendingPicks_; }
  int RerollsLeft() const { return rerollsLeft_; }
  void UseReroll() { if (rerollsLeft_ > 0) --rerollsLeft_; }
  // 코인 다시 뽑기: 15코인에서 시작해 이번 스테이지에서 낼 때마다 10씩 비싸진다
  int RerollCost() const { return 15 + 10 * paidRerolls_; }
  void PayReroll() { ++paidRerolls_; }
  void OnEnemyKilled();           // 영혼 흡수 카드: 12마리마다 하트 1
  int LevelUpFlash() const { return levelUpFlash_; }  // HUD 의 경험치 바가 번쩍이는 연출

  // 나이트메어 필살기: frames 동안 적/보스/적 탄환이 멈춘다. 끝날 때 화면 전체 피해.
  void StartTimeStop(int frames) { timeStop_ = frames; }
  bool TimeStopped() const { return timeStop_ > 0; }

  // ---- 타격감 ----
  // 히트스톱: frames 동안 월드 전체가 멈춘다 (화면 흔들림은 유지). 겹치면 긴 쪽을 따른다.
  void HitStop(int frames) { hitStop_ = std::max(hitStop_, frames); }
  // 콤보: 적을 처치할 때마다 +1, ComboFrames() 안에 다음 처치가 없거나 피격되면 끊긴다. (콤보 유지 카드로 늘어남)
  int ComboFrames() const;
  int RegisterKill(Vec2 at);  // 콤보 +1 후 점수 배율(1 + 콤보/5)을 돌려준다. 5콤보마다 코인 보너스.
  void BreakCombo() { combo_ = 0; comboTimer_ = 0; }
  int Combo() const { return combo_; }
  int ComboTimer() const { return comboTimer_; }
  int ComboPop() const { return comboPop_; }  // 콤보가 오른 직후 HUD 글자가 커지는 연출용
  int ComboMultiplier() const { return 1 + combo_ / 5; }

  // ---- 매복 결투장 ----
  bool ArenaActive() const { return arena_ >= 0; }
  int ArenaWave() const { return arenaWave_ + 1; }   // 지금 웨이브 (1부터)
  int ArenaWaves() const;
  int ArenaClock() const { return arenaClock_; }     // 이번 웨이브 남은 프레임 (0 = 제한 없음: 마지막 웨이브)
  int ArenaClockMax() const;
  bool ArenaSummoning() const { return arena_ >= 0 && arenaTimer_ > 0; }  // 다음 웨이브 소환 예고 중

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
  std::unordered_map<std::type_index, std::vector<std::unique_ptr<Actor>>> pools_;  // 타입별로 쉬고 있는 투사체
  int pooledCreated_ = 0;  // 풀이 비어서 새로 만든 수 (로그용)
  int pooledReused_ = 0;   // 풀에서 꺼내 재사용한 수 (로그용)
  Player* player_ = nullptr;
  Boss* boss_ = nullptr;
  int frame_ = 0;
  bool ultActive_ = false;
  int hitStop_ = 0;
  int combo_ = 0;
  int comboTimer_ = 0;
  int comboPop_ = 0;
  bool stageCleared_ = false;
  bool bossDefeated_ = false;
  bool hatCollected_ = false;
  std::string banner_;
  int bannerTimer_ = 0;
  float xpFrac_ = 0.f;     // 경험치 증가 카드의 소수점 경험치
  int pendingPicks_ = 0;   // 아직 고르지 않은 카드 화면 수
  int rerollsLeft_ = 0;    // 이번 스테이지에 남은 무료 다시 뽑기 (리롤 카드)
  int paidRerolls_ = 0;    // 이번 스테이지에 코인을 내고 다시 뽑은 수
  int kills_ = 0;          // 영혼 흡수 카운트
  int levelUpFlash_ = 0;
  int timeStop_ = 0;

  void EndTimeStop();
  void SpawnFrom(const SpawnPoint& sp, int arenaTag);  // 스폰 지점 하나를 실제 액터로
  void UpdateArenas();
  void SetArenaWalls(const ArenaDef& a, bool on);

  // 매복 결투장 진행 상태
  int arena_ = -1;               // 진행 중인 결투장 (-1 = 없음)
  int arenaWave_ = 0;            // 지금 싸우는(또는 곧 나올) 웨이브
  int arenaTimer_ = 0;           // >0: 다음 웨이브가 나오기까지 남은 프레임 (바닥에 소환 마법진)
  int arenaClock_ = 0;           // 웨이브 제한 시간: 0 이 되면 남은 적이 있어도 다음 웨이브(증원)가 나온다
  std::vector<bool> arenaDone_;  // 끝낸 결투장
};

}  // namespace gengar

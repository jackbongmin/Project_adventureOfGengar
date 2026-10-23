#pragma once
// 플레이어의 섀도우볼. pos = 중심 좌표. 사거리(life)가 끝나면 작아지며 사라진다.
// 날아가는 동안 지나온 자리에 잔상을 남기고, 발사 직후 출발점에 작은 도트 고리가 퍼진다.
// 카드 능력(관통, 튕김, 유도, 폭발, 분열, 저주, 치명타)은 ShotSpec 으로 받아서 모두 함께 적용한다.
#include "actor/Actor.h"

namespace gengar {

struct ShotSpec {
  float radius = 7.f;
  float damage = 1.f;
  int life = 36;
  int pierce = 0;          // 적을 맞혀도 사라지지 않고 더 지나갈 수 있는 수
  int bounces = 0;         // 벽/바닥에 닿아도 튕겨서 계속 가는 수
  bool homing = false;
  bool burst = false;      // 맞힌 자리 주변에 50% 폭발
  bool split = false;      // 맞힌 자리에서 작은 공 3개로 분열
  bool curse = false;      // 맞은 적이 3초 동안 받는 피해 +25%
  bool fragment = false;   // 분열 조각: 다시 폭발/분열하지 않는다 (연쇄 방지)
  bool charged = false;    // 차지샷 (크고 밝게 그린다)
  bool ultimate = false;   // 필살기 탄: 게이지 충전과 치명타 없음
  float crit = 0.f;        // 치명타(2배) 확률
  float knockback = 0.f;
  float bossScale = 1.f;   // 보스에게 주는 피해 배율
};

class ShadowBall : public Actor {
 public:
  ShadowBall(Vec2 center, Vec2 velocity, const ShotSpec& spec, const Actor* ignore = nullptr);
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;
  Rect Bounds() const override { return {pos.x - spec_.radius, pos.y - spec_.radius, spec_.radius * 2.f, spec_.radius * 2.f}; }

 private:
  static constexpr int kGhosts = 3;       // 잔상 개수
  static constexpr int kGhostStride = 2;  // 잔상 하나당 몇 프레임 전 위치인가
  static constexpr int kHistory = kGhosts * kGhostStride;
  static constexpr int kMaxHits = 12;

  void Steer(World& w);
  bool AlreadyHit(const Actor* a) const;
  void Hit(World& w, Actor& target);  // 피해 + 폭발/분열 처리

  ShotSpec spec_;
  int life_;
  int age_ = 0;
  Vec2 origin_;
  Vec2 history_[kHistory] = {};  // 최근 위치 (원형 버퍼, age_ 로 인덱스)
  const Actor* hits_[kMaxHits] = {};  // 이미 맞힌 대상 (관통 중 같은 적을 두 번 맞히지 않는다)
  int hitCount_ = 0;
};

}  // namespace gengar

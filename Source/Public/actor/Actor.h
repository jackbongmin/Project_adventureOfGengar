#pragma once
// 월드에 존재하는 모든 객체의 기반 클래스.
// ActorKind의 값 순서가 그대로 렌더링 레이어 순서(작을수록 뒤)이다.
#include "core/Types.h"

namespace gengar {

class World;
class Graphics;

enum class ActorKind { Pickup, Enemy, Boss, Hazard, Player, PlayerShot, EnemyShot };
constexpr int kActorKindCount = 7;

class Actor {
 public:
  explicit Actor(ActorKind kind) : kind_(kind) {}
  virtual ~Actor() = default;
  Actor(const Actor&) = delete;
  Actor& operator=(const Actor&) = delete;

  virtual void Update(World& world) = 0;
  virtual void Render(Graphics& gfx, const World& world) = 0;
  virtual Rect Bounds() const { return {pos.x, pos.y, size.x, size.y}; }

  ActorKind Kind() const { return kind_; }
  bool IsDead() const { return dead_; }  // 제거 예약됨
  void Kill() { dead_ = true; }

  Vec2 pos;   // 기본: 좌상단 (투사체/보스/픽업은 중심좌표 - 각 클래스에 명시)
  Vec2 vel;
  Vec2 size;

 private:
  ActorKind kind_;
  bool dead_ = false;
};

}  // namespace gengar

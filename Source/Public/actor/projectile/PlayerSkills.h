#pragma once
// 카드로 얻는 플레이어 기술 액터들.
//  - DashBlast  : 잔상 폭발 카드. 대시를 시작한 자리에 남은 잔상이 잠시 뒤 터진다.
//  - BlackHole  : 블랙홀 필살기. 적을 끌어당기며 계속 피해를 주고, 적 탄환을 빨아들인다.
//  - Familiar   : 그림자 분신 카드. 플레이어를 따라다니며 가까운 적에게 섀도우볼을 쏜다.
#include "actor/Actor.h"

namespace gengar {

class DashBlast : public Actor {
 public:
  DashBlast(Vec2 center, int facing, float damage, float radius);
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;

 private:
  static constexpr int kDelay = 30;  // 0.5초 뒤 폭발
  int facing_;
  float damage_;
  float radius_;
  int t_ = 0;
};

class BlackHole : public Actor {
 public:
  BlackHole(Vec2 center, float enemyDamage, float bossDamage);
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;

 private:
  static constexpr int kLife = 240;       // 4초
  static constexpr int kTickFrames = 15;  // 0.25초마다 피해
  float enemyDamage_;
  float bossDamage_;
  int t_ = 0;
};

class Familiar : public Actor {
 public:
  explicit Familiar(Vec2 center);
  void Update(World& world) override;
  void Render(Graphics& gfx, const World& world) override;

 private:
  static constexpr int kFireFrames = 48;  // 0.8초마다 발사
  int cooldown_ = kFireFrames;
  int facing_ = 1;
  int t_ = 0;
};

}  // namespace gengar

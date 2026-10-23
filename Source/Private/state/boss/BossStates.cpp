#include "state/boss/BossStates.h"

#include <cmath>

#include "actor/Player.h"
#include "actor/enemy/Bat.h"
#include "actor/enemy/Enemy.h"
#include "actor/hazard/PillarHazard.h"
#include "actor/pickup/Pickups.h"
#include "actor/projectile/Shockwave.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "core/Random.h"
#include "core/Session.h"
#include "data/BossTuning.h"
#include "platform/Audio.h"
#include "world/World.h"

namespace gengar {
namespace {

constexpr float kW = static_cast<float>(cfg::kScreenW);
constexpr float kGroundY = static_cast<float>(cfg::kGroundRow * cfg::kTile);

// 모든 보스 상태가 공유하는 카운터. 각 상태는 Update 시작 시 ++t_ 한다.
// 패턴의 수치(탄 수, 속도, 예고 시간 등)는 data/BossTuning.cpp 의 단계별 표에서 읽는다.
class BossStateBase : public BossState {
 protected:
  int t_ = 0;    // 상태 진입 후 경과 프레임
  int n_ = 0;    // 반복 횟수 등 보조 카운터
  int sub_ = 0;  // 하위 단계
};

// 등장: 위에서 내려오며 서서히 나타난다 (무적)
class EnterState : public BossStateBase {
 public:
  const char* Name() const override { return "Enter"; }
  void Enter(Boss& b) override { b.invulnerable = true; b.contactEnabled = false; b.alpha = 0.f; }
  void Update(Boss& b) override {
    ++t_;
    b.pos.y += (360.f - b.pos.y) * 0.04f;
    b.alpha = std::min(1.f, static_cast<float>(t_) / 60.f);
    b.GetWorld().Fx().Shake(2.f);
    if (t_ > 140) { DebugLog("[boss] fight start"); b.ToFloat(); }
  }
};

// 기본: 플레이어와 거리를 두고 떠다니다가 다음 공격을 고른다
class FloatState : public BossStateBase {
 public:
  const char* Name() const override { return "Float"; }
  void Update(Boss& b) override {
    ++t_;
    b.FloatMove();
    if (t_ > Tuning(b.Phase()).idleFrames) {
      const std::vector<BossAttack>& seq = AttackSequence(b.Phase());
      // 단계의 패턴 목록에서 무작위로 고른다. 직전과 같은 패턴이 연속으로 나오지는 않는다.
      const int last = b.attackIndex, n = static_cast<int>(seq.size());
      int pick = RandInt(0, n - 1);
      if (n > 1 && pick == last) pick = (pick + 1 + RandInt(0, n - 2)) % n;
      b.attackIndex = pick;
      b.StartAttack(seq[static_cast<size_t>(pick)]);
    }
  }
};

// 암흑탄: 기를 모은 뒤 플레이어를 향해 부채꼴로 연속 발사
class VolleyState : public BossStateBase {
 public:
  const char* Name() const override { return "Volley"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    const BossTuning& T = Tuning(b.Phase());
    b.FloatMove();
    b.charge = t_ < 24 ? static_cast<float>(t_) / 24.f : 0.f;
    if (t_ == 24) {
      const Vec2 pc = b.PlayerCenter();
      const float base = std::atan2(pc.y - b.pos.y, pc.x - b.pos.x);
      for (int i = 0; i < T.volleyCount; ++i) {
        b.FireOrb(b.pos, base + (static_cast<float>(i) - static_cast<float>(T.volleyCount - 1) / 2.f) * 0.24f, T.volleySpeed);
      }
      ++n_;
      w.Snd().Play(Sfx::BossShoot);
    }
    if (t_ >= 46) {
      if (n_ < T.volleyRepeats) t_ = 0;
      else b.ToFloat();
    }
  }
};

// 공허의 고리: 전방위 원형 탄막을 여러 겹으로 (겹마다 엇갈리게)
class RingState : public BossStateBase {
 public:
  const char* Name() const override { return "Ring"; }
  void Update(Boss& b) override {
    ++t_;
    const BossTuning& T = Tuning(b.Phase());
    constexpr int kFirst = 34, kGap = 40;
    constexpr float kRingGap = 0.5f;  // 플레이어 방향 ±0.5rad(약 ±29도)는 비운다
    b.FloatMove();
    b.charge = t_ < kFirst ? static_cast<float>(t_) / static_cast<float>(kFirst) : 0.f;
    if (t_ >= kFirst && (t_ - kFirst) % kGap == 0 && (t_ - kFirst) / kGap < T.ringWaves) {
      const int wave = (t_ - kFirst) / kGap;
      const float offset = wave % 2 == 0 ? 0.f : kPi / static_cast<float>(T.ringCount);
      // 플레이어 쪽으로 한 군데 틈을 비워 둔다: 틈으로 빠져나가면 맞지 않는다
      const Vec2 pc = b.PlayerCenter();
      const float aim = std::atan2(pc.y - b.pos.y, pc.x - b.pos.x);
      for (int i = 0; i < T.ringCount; ++i) {
        const float a = offset + static_cast<float>(i) * 2.f * kPi / static_cast<float>(T.ringCount);
        if (std::fabs(std::remainder(a - aim, 2.f * kPi)) < kRingGap) continue;
        b.FireOrb(b.pos, a, T.ringSpeed);
      }
      b.GetWorld().Snd().Play(Sfx::BossRoar);
    }
    if (t_ >= kFirst + (T.ringWaves - 1) * kGap + 36) b.ToFloat();
  }
};

// 소환: 양쪽에서 박쥐 2마리 (동시에 4마리 이상이면 생략)
class SummonState : public BossStateBase {
 public:
  const char* Name() const override { return "Summon"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    b.FloatMove();
    b.charge = std::min(1.f, static_cast<float>(t_) / 32.f);
    if (t_ == 32) {
      if (w.CountAlive(ActorKind::Enemy) < 4) {
        w.Spawn<Bat>(Vec2{200.f, 140.f}, w.StageIndex(), 180.f);
        w.Spawn<Bat>(Vec2{kW - 230.f, 140.f}, w.StageIndex(), 180.f);
        w.Fx().Burst({200.f, 140.f}, 14, Hex(0xff6a8a), 4.f);
        w.Fx().Burst({kW - 230.f, 140.f}, 14, Hex(0xff6a8a), 4.f);
        w.Snd().Play(Sfx::BossSummon);
      }
      b.charge = 0.f;
    }
    if (t_ >= 56) b.ToFloat();
  }
};

// 내려찍기: 플레이어 위로 이동 -> 급강하 -> 좌우 충격파. 착지 후 빈틈(피해 1.5배)
class SlamState : public BossStateBase {
 public:
  const char* Name() const override { return "Slam"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    const BossTuning& T = Tuning(b.Phase());
    if (sub_ == 0) {
      b.pos.x += Clamp((b.PlayerCenter().x - b.pos.x) * 0.1f, -8.f, 8.f);
      b.pos.y += (150.f - b.pos.y) * 0.12f;
      b.charge = std::min(1.f, static_cast<float>(t_) / static_cast<float>(T.slamWindup));
      if (t_ >= T.slamWindup) { sub_ = 1; fall_ = 0.f; b.charge = 0.f; b.contactEnabled = true; }  // 낙하 중에만 접촉 피해
    } else if (sub_ == 1) {
      fall_ += 1.1f;
      b.pos.y += fall_;
      if (b.pos.y + 52.f >= kGroundY) {
        b.pos.y = kGroundY - 52.f;
        sub_ = 2;
        t_ = 0;
        b.contactEnabled = false;
        b.damageMultiplier = 1.5f;
        w.Fx().Shake(16.f);
        w.Snd().Play(Sfx::BossSlam);
        w.Fx().Burst({b.pos.x, kGroundY}, 24, Hex(0xd9a0ff), 5.f);
        for (int s = -1; s <= 1; s += 2) {
          w.SpawnPooled<Shockwave>(Vec2{b.pos.x + static_cast<float>(s) * 60.f, kGroundY - 11.f}, static_cast<float>(s) * T.slamWaveSpeed);
        }
      }
    } else if (t_ >= T.slamRest) {
      b.ToFloat();
    }
  }

 private:
  float fall_ = 0.f;
};

// 공허 광선: 조준선 예고 -> 굵은 광선 (2단계부터 서서히 회전, 3단계에서 더 빠르게)
class BeamState : public BossStateBase {
 public:
  const char* Name() const override { return "Beam"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    const BossTuning& T = Tuning(b.Phase());
    const Vec2 pc = b.PlayerCenter();
    constexpr int kLockAt = 18;
    b.FloatMove();
    if (t_ < kLockAt) {
      b.charge = static_cast<float>(t_) / static_cast<float>(kLockAt);
    } else if (t_ == kLockAt) {
      b.beam = BeamInfo{true, false, std::atan2(pc.y - b.pos.y, pc.x - b.pos.x), Chance(0.5f) ? -1 : 1};
      w.Snd().Play(Sfx::BeamCharge);
    } else if (t_ == T.beamFireAt && b.beam.active) {
      b.beam.firing = true;
      w.Fx().Shake(8.f);
      w.Snd().Play(Sfx::BeamFire);
    }
    if (b.beam.active && b.beam.firing) {
      b.beam.angle += static_cast<float>(b.beam.sweepDir) * T.beamSweep;
      const float dx = pc.x - b.pos.x, dy = pc.y - b.pos.y;
      const float ca = std::cos(b.beam.angle), sa = std::sin(b.beam.angle);
      Player& p = w.GetPlayer();
      if (p.IsAlive() && dx * ca + dy * sa > 0.f && std::fabs(dx * sa - dy * ca) < 17.f) {
        p.TakeDamage(w, DamageInfo{1.f, Sign(dx) != 0 ? Sign(dx) : 1, false, false, "beam"});
      }
    }
    if (t_ >= T.beamFireAt + 30) b.ToFloat();
  }
};

// 어둠의 비: 천장에서 마법구가 연달아 떨어진다 (3발 중 2발은 플레이어 위치를 노린다)
class RainState : public BossStateBase {
 public:
  const char* Name() const override { return "Rain"; }
  void Update(Boss& b) override {
    ++t_;
    const BossTuning& T = Tuning(b.Phase());
    constexpr int kStart = 24;
    b.FloatMove();
    b.charge = t_ < kStart ? static_cast<float>(t_) / static_cast<float>(kStart) : 0.f;
    if (t_ >= kStart && t_ < kStart + T.rainFrames && (t_ - kStart) % T.rainInterval == 0) {
      const float px = b.PlayerCenter().x;
      const float x = n_ % 3 == 0 ? RandRange(60.f, kW - 60.f) : Clamp(px + RandRange(-100.f, 100.f), 50.f, kW - 50.f);
      b.FireOrb({x, 20.f}, kPi / 2.f, T.rainSpeed);
      ++n_;
    }
    if (t_ >= kStart + T.rainFrames + 30) b.ToFloat();
  }
};

// 그림자 기둥: 플레이어 주변 바닥에 마법진 예고 후 기둥 분출 (여러 번 연속)
class PillarState : public BossStateBase {
 public:
  const char* Name() const override { return "Pillar"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    const BossTuning& T = Tuning(b.Phase());
    constexpr int kFirst = 26, kGap = 44;
    b.FloatMove();
    b.charge = t_ < kFirst ? static_cast<float>(t_) / static_cast<float>(kFirst) : 0.f;
    if (t_ >= kFirst && (t_ - kFirst) % kGap == 0 && (t_ - kFirst) / kGap < T.pillarWaves) {
      const float px = b.PlayerCenter().x;
      for (int i = 0; i < T.pillarCount; ++i) {
        const float x = px + (static_cast<float>(i) - static_cast<float>(T.pillarCount - 1) / 2.f) * 150.f;  // 기둥 사이 간격: 사이로 지나갈 수 있게 넓게
        w.Spawn<PillarHazard>(Clamp(x, 50.f, kW - 50.f), T.pillarWarn);
      }
      w.Snd().Play(Sfx::PillarWarn);
    }
    if (t_ >= kFirst + (T.pillarWaves - 1) * kGap + T.pillarWarn + 40) b.ToFloat();
  }
};

// 바닥 질주: 반대편 벽으로 이동해 기를 모은 뒤 바닥을 가로질러 달린다. 끝난 뒤 빈틈(1.5배)
class DashState : public BossStateBase {
 public:
  const char* Name() const override { return "Dash"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    const BossTuning& T = Tuning(b.Phase());
    if (t_ == 1) {
      const float px = b.PlayerCenter().x;
      targetX_ = px < kW / 2.f ? kW - 100.f : 100.f;
      dashV_ = (targetX_ > kW / 2.f ? -1.f : 1.f) * T.dashSpeed;
    }
    if (sub_ == 0) {
      b.pos.x += Clamp((targetX_ - b.pos.x) * 0.1f, -10.f, 10.f);
      b.pos.y += (424.f - b.pos.y) * 0.12f;
      b.charge = std::min(1.f, static_cast<float>(t_) / static_cast<float>(T.dashWindup));
      if ((t_ >= T.dashWindup && std::fabs(b.pos.x - targetX_) < 14.f) || t_ >= T.dashWindup + 50) {
        sub_ = 1; t_ = 0; b.charge = 0.f; b.contactEnabled = true;  // 질주 중에만 접촉 피해
        w.Snd().Play(Sfx::BossDash);
      }
    } else if (sub_ == 1) {
      b.pos.x += dashV_;
      w.Fx().Shake(3.f);
      if (t_ % 2 == 0) w.Fx().Burst({b.pos.x, b.pos.y + 40.f}, 3, Hex(0xd9a0ff), 3.f, 16.f);
      if (b.pos.x <= 100.f || b.pos.x >= kW - 100.f) {
        b.pos.x = Clamp(b.pos.x, 100.f, kW - 100.f);
        sub_ = 2; t_ = 0;
        b.contactEnabled = false;
        b.damageMultiplier = 1.5f;
        w.Fx().Shake(14.f);
        w.Snd().Play(Sfx::BossSlam);
      }
    } else if (t_ >= T.dashRest) {
      b.ToFloat();
    }
  }

 private:
  float targetX_ = 0.f, dashV_ = 0.f;
};

// 순간이동: 사라졌다가 반대편에 나타나 부채꼴 탄을 여러 번 쏜다 (사라지는 동안 무적)
class BlinkState : public BossStateBase {
 public:
  const char* Name() const override { return "Blink"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    const BossTuning& T = Tuning(b.Phase());
    const Vec2 pc = b.PlayerCenter();
    constexpr int kFirst = 26, kGap = 22;
    if (sub_ == 0) {
      b.invulnerable = true;
      b.alpha = std::max(0.f, b.alpha - 0.09f);
      if (b.alpha <= 0.f) {
        sub_ = 1; t_ = 0;
        b.pos.x = Clamp(kW - pc.x, 110.f, kW - 110.f);
        if (std::fabs(b.pos.x - pc.x) < 160.f) b.pos.x = pc.x < kW / 2.f ? kW - 110.f : 110.f;
        b.pos.y = 240.f;
        w.Fx().Burst(b.pos, 20, Hex(0xffb0ff), 5.f);
        w.Snd().Play(Sfx::BossBlink);
      }
    } else {
      b.alpha = std::min(1.f, b.alpha + 0.09f);
      if (b.alpha >= 1.f) b.invulnerable = false;
      b.charge = t_ < kFirst ? static_cast<float>(t_) / static_cast<float>(kFirst) : 0.f;
      if (t_ >= kFirst && (t_ - kFirst) % kGap == 0 && (t_ - kFirst) / kGap < T.blinkWaves) {
        const float base = std::atan2(pc.y - b.pos.y, pc.x - b.pos.x);
        const int half = T.blinkCount / 2;
        for (int i = -half; i <= half; ++i) b.FireOrb(b.pos, base + static_cast<float>(i) * 0.26f, T.blinkSpeed);
        w.Snd().Play(Sfx::BossShoot);
      }
      if (t_ >= kFirst + (T.blinkWaves - 1) * kGap + 30) b.ToFloat();
    }
  }
};

// 소용돌이 탄막: 멀리서 회전하는 탄 줄기를 연달아 뿌린다
class SpiralState : public BossStateBase {
 public:
  const char* Name() const override { return "Spiral"; }
  void Update(Boss& b) override {
    ++t_;
    const BossTuning& T = Tuning(b.Phase());
    constexpr int kStart = 26;
    b.FloatMove();
    b.charge = t_ < kStart ? static_cast<float>(t_) / static_cast<float>(kStart) : 0.f;
    if (t_ >= kStart && t_ < kStart + T.spiralFrames && (t_ - kStart) % T.spiralInterval == 0) {
      const float base = static_cast<float>(t_) * 0.15f;
      for (int i = 0; i < T.spiralArms; ++i) b.FireOrb(b.pos, base + static_cast<float>(i) * 2.f * kPi / static_cast<float>(T.spiralArms), T.spiralSpeed);
      if ((t_ - kStart) % 20 == 0) b.GetWorld().Snd().Play(Sfx::BossShoot);
    }
    if (t_ >= kStart + T.spiralFrames + 36) b.ToFloat();
  }
};

// 유도탄: 플레이어를 향해 휘어지며 따라오는 구슬. 일정 시간 뒤 사라지므로 빙 돌아 피하면 된다.
class HomingState : public BossStateBase {
 public:
  const char* Name() const override { return "Homing"; }
  void Update(Boss& b) override {
    ++t_;
    const BossTuning& T = Tuning(b.Phase());
    constexpr int kFirst = 32, kGap = 30;
    b.FloatMove();
    b.charge = t_ < kFirst ? static_cast<float>(t_) / static_cast<float>(kFirst) : 0.f;
    if (t_ >= kFirst && (t_ - kFirst) % kGap == 0 && (t_ - kFirst) / kGap < T.homingWaves) {
      const Vec2 pc = b.PlayerCenter();
      const float base = std::atan2(pc.y - b.pos.y, pc.x - b.pos.x);
      for (int i = 0; i < T.homingCount; ++i) {
        b.FireHomingOrb(b.pos, base + (static_cast<float>(i) - static_cast<float>(T.homingCount - 1) / 2.f) * 0.55f, T.homingSpeed, T.homingTurn, 150);
      }
      b.GetWorld().Snd().Play(Sfx::BossShoot);
    }
    if (t_ >= kFirst + (T.homingWaves - 1) * kGap + 30) b.ToFloat();
  }
};

// 돌격: 기를 모으며 조준선으로 플레이어 위치를 고정해 보여 준 뒤(예고), 그 선을 따라 몸으로 달려든다.
// 바닥에 닿으면 바닥을 따라 조금만(약 90px) 미끄러지다 멈추고, 그 뒤 잠시 빈틈이 생긴다(피해 1.5배).
// 단계가 오를수록 예고가 짧고 빠르며, 연속 횟수가 늘어난다.
class LungeState : public BossStateBase {
 public:
  const char* Name() const override { return "Lunge"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    const BossTuning& T = Tuning(b.Phase());
    constexpr int kLockAt = 14;
    if (sub_ == 0) {
      if (t_ < kLockAt) b.FloatMove();  // 조준을 고정하기 전까지는 천천히 떠 있다
      b.charge = std::min(1.f, static_cast<float>(t_) / static_cast<float>(T.lungeWindup));
      if (t_ == kLockAt) {  // 조준 고정: 이후 이 선을 따라 돌진한다
        const Vec2 pc = b.PlayerCenter();
        angle_ = std::atan2(pc.y - b.pos.y, pc.x - b.pos.x);
        b.beam = BeamInfo{true, false, angle_, 1};
        w.Snd().Play(Sfx::BeamCharge);
      }
      if (t_ >= T.lungeWindup) {
        sub_ = 1; t_ = 0; traveled_ = 0.f; floorDist_ = 0.f; onFloor_ = false;
        b.charge = 0.f;
        b.beam.active = false;
        b.contactEnabled = true;  // 돌진 중에만 접촉 피해
        w.Snd().Play(Sfx::BossDash);
      }
    } else if (sub_ == 1) {
      const float speed = T.lungeSpeed;
      float vx = std::cos(angle_) * speed, vy = std::sin(angle_) * speed;
      if (onFloor_) { vx = (vx >= 0.f ? 1.f : -1.f) * speed; vy = 0.f; floorDist_ += speed; }  // 바닥을 따라 잠깐만 미끄러진다
      b.pos.x += vx;
      b.pos.y += vy;
      traveled_ += speed;
      if (t_ % 2 == 0) w.Fx().Burst(b.pos, 2, Hex(0xd9a0ff), 3.f, 14.f);
      w.Fx().Shake(2.f);
      if (b.pos.y + 52.f >= kGroundY) { b.pos.y = kGroundY - 52.f; onFloor_ = true; }
      const bool wall = b.pos.x <= 100.f || b.pos.x >= kW - 100.f, ceiling = b.pos.y < 70.f, tooFar = traveled_ > 950.f, slid = floorDist_ > kMaxSlide;
      if (wall || ceiling || tooFar || slid) {
        b.pos.x = Clamp(b.pos.x, 100.f, kW - 100.f);
        b.pos.y = std::max(b.pos.y, 70.f);
        sub_ = 2; t_ = 0;
        b.contactEnabled = false;
        b.damageMultiplier = 1.5f;
        w.Fx().Shake(12.f);
        w.Snd().Play(Sfx::BossSlam);
      }
    } else if (t_ >= T.lungeRest) {
      if (n_ < T.lungeExtra) { ++n_; sub_ = 0; t_ = 0; b.damageMultiplier = 1.f; }
      else b.ToFloat();
    }
  }

 private:
  static constexpr float kMaxSlide = 90.f;  // 바닥에 닿은 뒤 더 미끄러지는 최대 거리(px)
  float angle_ = 0.f, traveled_ = 0.f, floorDist_ = 0.f;
  bool onFloor_ = false;
};

// 십자포화: 양쪽 벽에서 가로 탄 줄이 번갈아 날아온다. 줄마다 한 칸은 비어 있어 그 틈으로 지나가면 된다.
class CrossfireState : public BossStateBase {
 public:
  const char* Name() const override { return "Crossfire"; }
  void Update(Boss& b) override {
    ++t_;
    const BossTuning& T = Tuning(b.Phase());
    constexpr int kFirst = 30, kGap = 42;
    constexpr float kRowGap = 52.f;
    b.FloatMove();
    b.charge = t_ < kFirst ? static_cast<float>(t_) / static_cast<float>(kFirst) : 0.f;
    if (t_ >= kFirst && (t_ - kFirst) % kGap == 0 && (t_ - kFirst) / kGap < T.crossWaves) {
      const int wave = (t_ - kFirst) / kGap;
      const bool fromLeft = wave % 2 == 0;
      const float top = kGroundY - 30.f - kRowGap * static_cast<float>(T.crossRows - 1);
      const int hole = RandInt(0, T.crossRows - 1);
      for (int i = 0; i < T.crossRows; ++i) {
        if (i == hole) continue;
        b.FireOrb({fromLeft ? -20.f : kW + 20.f, top + kRowGap * static_cast<float>(i)}, fromLeft ? 0.f : kPi, T.crossSpeed);
      }
      b.GetWorld().Snd().Play(Sfx::BossShoot);
    }
    if (t_ >= kFirst + (T.crossWaves - 1) * kGap + 50) b.ToFloat();
  }
};

// 허공 휩쓸기: 조준한 방향을 중심으로 탄 다발이 좌우로 부채꼴을 훑는다.
class SweepState : public BossStateBase {
 public:
  const char* Name() const override { return "Sweep"; }
  void Update(Boss& b) override {
    ++t_;
    const BossTuning& T = Tuning(b.Phase());
    constexpr int kStart = 26;
    b.FloatMove();
    b.charge = t_ < kStart ? static_cast<float>(t_) / static_cast<float>(kStart) : 0.f;
    if (t_ == kStart) {
      const Vec2 pc = b.PlayerCenter();
      center_ = std::atan2(pc.y - b.pos.y, pc.x - b.pos.x);
    }
    if (t_ >= kStart && t_ < kStart + T.sweepFrames && (t_ - kStart) % T.sweepInterval == 0) {
      const float phase = static_cast<float>(t_ - kStart) / static_cast<float>(T.sweepFrames) * 2.f * kPi;
      const float base = center_ + std::sin(phase) * T.sweepArc;
      for (int i = 0; i < T.sweepCount; ++i) {
        b.FireOrb(b.pos, base + (static_cast<float>(i) - static_cast<float>(T.sweepCount - 1) / 2.f) * 0.3f, T.sweepSpeed);
      }
      if ((t_ - kStart) % 20 == 0) b.GetWorld().Snd().Play(Sfx::BossShoot);
    }
    if (t_ >= kStart + T.sweepFrames + 30) b.ToFloat();
  }

 private:
  float center_ = 0.f;
};

// 사망: 연쇄 폭발 -> 쥐고 있던 보물 모자를 플레이어 쪽으로 내밀어 놓아준다 -> 흩어지며 사라진다.
// 완전히 사라진 뒤 보스 격파를 알리고, 모자를 주우면 엔딩으로 넘어간다 (TreasureHat).
class DyingState : public BossStateBase {
 public:
  const char* Name() const override { return "Dying"; }
  void Enter(Boss& b) override {
    b.dying = true; b.invulnerable = true; b.contactEnabled = false; b.charge = 0.f; b.beam.active = false;
  }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    if (t_ <= kExplodeEnd) {
      w.Fx().Shake(6.f);
      b.pos.y += 0.3f;
      if (t_ % 6 == 0) {
        const Color colors[3] = {Hex(0xffb0ff), Hex(0xff6aa0), Hex(0xffffff)};
        w.Fx().Burst({b.pos.x + RandRange(-50.f, 50.f), b.pos.y + RandRange(-50.f, 50.f)}, 14, colors[(t_ / 6) % 3], 5.f, 40.f);
        w.Snd().Tone(RandRange(80.f, 200.f), RandRange(40.f, 80.f), 0.2f, Wave::Saw, 0.05f);
      }
      if (t_ == kExplodeEnd) {
        w.Fx().Burst(b.pos, 60, Hex(0xffd0ff), 8.f, 60.f);
        w.Fx().Pop({b.pos.x, b.pos.y - 80.f}, "...TAKE IT BACK.", Hex(0xffe27a));
        // 플레이어 쪽으로, 화면 안쪽에 모자를 내민다
        const Vec2 pc = b.PlayerCenter();
        const float dir = pc.x < b.pos.x ? -1.f : 1.f;
        b.handTarget = {Clamp(b.pos.x + dir * 120.f, 80.f, kW - 80.f), b.pos.y + 10.f};
      }
      return;
    }
    if (t_ <= kRelease) {  // 손을 뻗어 모자를 내민다
      b.handReach = EaseOut(static_cast<float>(t_ - kExplodeEnd) / static_cast<float>(kRelease - kExplodeEnd));
      if (t_ == kRelease) {
        const Vec2 h = b.HatHandPos();
        b.holdingHat = false;
        w.Spawn<TreasureHat>(Vec2{h.x, h.y + 16.f});
        w.Snd().Play(Sfx::Heart);
        DebugLog("[boss] released the treasure hat");
      }
      return;
    }
    // 흩어지며 사라진다
    const float k = static_cast<float>(t_ - kRelease) / static_cast<float>(kVanish - kRelease);
    b.handReach = 1.f - k;
    b.alpha = 1.f - k;
    b.pos.y -= 0.4f;
    if (t_ % 4 == 0) w.Fx().Burst({b.pos.x + RandRange(-50.f, 50.f), b.pos.y + RandRange(-40.f, 50.f)}, 3, Hex(0xb46cff), 1.5f, 50.f);
    if (t_ >= kVanish) {
      w.Fx().Burst(b.pos, 40, Hex(0xffd0ff), 6.f, 60.f);
      Session& s = w.Sess();
      s.score += 5000 + s.lives * 1000 + w.GetPlayer().Hp() * 200;
      DebugLog("[boss] DEFEATED");
      w.NotifyBossDefeated();
      b.Kill();
    }
  }

 private:
  static constexpr int kExplodeEnd = 140, kRelease = 190, kVanish = 260;
  static float EaseOut(float k) { k = Clamp(k, 0.f, 1.f); return 1.f - (1.f - k) * (1.f - k); }
};

}  // namespace

std::unique_ptr<BossState> MakeBossEnterState() { return std::make_unique<EnterState>(); }
std::unique_ptr<BossState> MakeBossFloatState() { return std::make_unique<FloatState>(); }
std::unique_ptr<BossState> MakeBossDyingState() { return std::make_unique<DyingState>(); }

std::unique_ptr<BossState> MakeBossAttackState(BossAttack attack) {
  switch (attack) {
    case BossAttack::Volley: return std::make_unique<VolleyState>();
    case BossAttack::Ring: return std::make_unique<RingState>();
    case BossAttack::Summon: return std::make_unique<SummonState>();
    case BossAttack::Slam: return std::make_unique<SlamState>();
    case BossAttack::Beam: return std::make_unique<BeamState>();
    case BossAttack::Rain: return std::make_unique<RainState>();
    case BossAttack::Pillar: return std::make_unique<PillarState>();
    case BossAttack::Dash: return std::make_unique<DashState>();
    case BossAttack::Blink: return std::make_unique<BlinkState>();
    case BossAttack::Spiral: return std::make_unique<SpiralState>();
    case BossAttack::Homing: return std::make_unique<HomingState>();
    case BossAttack::Lunge: return std::make_unique<LungeState>();
    case BossAttack::Crossfire: return std::make_unique<CrossfireState>();
    case BossAttack::Sweep: return std::make_unique<SweepState>();
  }
  return std::make_unique<FloatState>();
}

}  // namespace gengar

#include "state/boss/BossStates.h"

#include <cmath>

#include "actor/Player.h"
#include "actor/enemy/Bat.h"
#include "actor/enemy/Enemy.h"
#include "actor/hazard/PillarHazard.h"
#include "actor/projectile/Shockwave.h"
#include "core/Config.h"
#include "core/Random.h"
#include "core/Session.h"
#include "platform/Audio.h"
#include "world/World.h"

namespace gengar {
namespace {

constexpr float kW = static_cast<float>(cfg::kScreenW);
constexpr float kGroundY = static_cast<float>(cfg::kGroundRow * cfg::kTile);

// 모든 보스 상태가 공유하는 카운터. 각 상태는 Update 시작 시 ++t_ 한다.
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
    if (t_ > 140) b.ToFloat();
  }
};

// 기본: 플레이어 주변을 떠다니다가 다음 공격을 고른다
class FloatState : public BossStateBase {
 public:
  const char* Name() const override { return "Float"; }
  void Update(Boss& b) override {
    ++t_;
    b.FloatMove();
    if (t_ > 90 - b.Phase() * 15) {
      const std::vector<BossAttack>& seq = AttackSequence(b.Phase());
      b.StartAttack(seq[static_cast<size_t>(b.attackIndex++) % seq.size()]);
    }
  }
};

// 암흑탄: 기를 모은 뒤 플레이어를 향해 부채꼴 발사
class VolleyState : public BossStateBase {
 public:
  const char* Name() const override { return "Volley"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    const int ph = b.Phase();
    b.FloatMove();
    b.charge = t_ < 30 ? static_cast<float>(t_) / 30.f : 0.f;
    if (t_ == 30) {
      const int count = ph == 2 ? 5 : 3;
      const Vec2 pc = b.PlayerCenter();
      const float base = std::atan2(pc.y - b.pos.y, pc.x - b.pos.x);
      for (int i = 0; i < count; ++i) {
        b.FireOrb(b.pos, base + (static_cast<float>(i) - static_cast<float>(count - 1) / 2.f) * 0.28f, 4.f + static_cast<float>(ph) * 0.5f);
      }
      ++n_;
      w.Snd().Play(Sfx::BossShoot);
    }
    if (t_ >= 60) {
      if (n_ < (ph >= 1 ? 2 : 1)) t_ = 0;
      else b.ToFloat();
    }
  }
};

// 공허의 고리: 전방위 원형 탄막 2회
class RingState : public BossStateBase {
 public:
  const char* Name() const override { return "Ring"; }
  void Update(Boss& b) override {
    ++t_;
    b.FloatMove();
    b.charge = std::min(1.f, static_cast<float>(t_ % 80) / 40.f);
    if (t_ == 40 || t_ == 80) {
      constexpr int kCount = 14;
      const float offset = t_ == 40 ? 0.f : kPi / kCount;
      for (int i = 0; i < kCount; ++i) b.FireOrb(b.pos, offset + static_cast<float>(i) * 2.f * kPi / kCount, 2.8f);
      b.charge = 0.f;
      b.GetWorld().Snd().Play(Sfx::BossRoar);
    }
    if (t_ >= 100) b.ToFloat();
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
    b.charge = std::min(1.f, static_cast<float>(t_) / 40.f);
    if (t_ == 40) {
      if (w.CountAlive(ActorKind::Enemy) < 4) {
        w.Spawn<Bat>(Vec2{200.f, 140.f}, w.StageIndex(), 180.f);
        w.Spawn<Bat>(Vec2{kW - 230.f, 140.f}, w.StageIndex(), 180.f);
        w.Fx().Burst({200.f, 140.f}, 14, Hex(0xff6a8a), 4.f);
        w.Fx().Burst({kW - 230.f, 140.f}, 14, Hex(0xff6a8a), 4.f);
        w.Snd().Play(Sfx::BossSummon);
      }
      b.charge = 0.f;
    }
    if (t_ >= 70) b.ToFloat();
  }
};

// 내려찍기: 플레이어 위로 이동 -> 급강하 -> 좌우 충격파. 착지 후 빈틈(피해 1.5배)
class SlamState : public BossStateBase {
 public:
  const char* Name() const override { return "Slam"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    const int ph = b.Phase();
    if (sub_ == 0) {
      b.pos.x += Clamp((b.PlayerCenter().x - b.pos.x) * 0.08f, -6.f, 6.f);
      b.pos.y += (150.f - b.pos.y) * 0.1f;
      b.charge = std::min(1.f, static_cast<float>(t_) / 50.f);
      if (t_ >= 50) { sub_ = 1; fall_ = 0.f; b.charge = 0.f; }
    } else if (sub_ == 1) {
      fall_ += 0.9f;
      b.pos.y += fall_;
      if (b.pos.y + 52.f >= kGroundY) {
        b.pos.y = kGroundY - 52.f;
        sub_ = 2;
        t_ = 0;
        b.damageMultiplier = 1.5f;
        w.Fx().Shake(16.f);
        w.Snd().Play(Sfx::BossSlam);
        w.Fx().Burst({b.pos.x, kGroundY}, 24, Hex(0xd9a0ff), 5.f);
        for (int s = -1; s <= 1; s += 2) {
          w.Spawn<Shockwave>(Vec2{b.pos.x + static_cast<float>(s) * 60.f, kGroundY - 11.f}, static_cast<float>(s) * (4.5f + static_cast<float>(ph) * 0.7f));
        }
      }
    } else if (t_ >= 55) {
      b.ToFloat();
    }
  }

 private:
  float fall_ = 0.f;
};

// 공허 광선: 조준선 예고 -> 굵은 광선 (3단계에서는 회전)
class BeamState : public BossStateBase {
 public:
  const char* Name() const override { return "Beam"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    const Vec2 pc = b.PlayerCenter();
    b.FloatMove();
    if (t_ < 20) {
      b.charge = static_cast<float>(t_) / 20.f;
    } else if (t_ == 20) {
      b.beam = BeamInfo{true, false, std::atan2(pc.y - b.pos.y, pc.x - b.pos.x), Chance(0.5f) ? -1 : 1};
      w.Snd().Play(Sfx::BeamCharge);
    } else if (t_ == 65 && b.beam.active) {
      b.beam.firing = true;
      w.Fx().Shake(8.f);
      w.Snd().Play(Sfx::BeamFire);
    }
    if (b.beam.active && b.beam.firing) {
      if (b.Phase() >= 2) b.beam.angle += static_cast<float>(b.beam.sweepDir) * 0.018f;
      const float dx = pc.x - b.pos.x, dy = pc.y - b.pos.y;
      const float ca = std::cos(b.beam.angle), sa = std::sin(b.beam.angle);
      Player& p = w.GetPlayer();
      if (p.IsAlive() && dx * ca + dy * sa > 0.f && std::fabs(dx * sa - dy * ca) < 17.f) {
        p.TakeDamage(w, DamageInfo{1.f, Sign(dx) != 0 ? Sign(dx) : 1});
      }
    }
    if (t_ >= 95) b.ToFloat();
  }
};

// 어둠의 비: 천장에서 마법구가 연달아 떨어진다
class RainState : public BossStateBase {
 public:
  const char* Name() const override { return "Rain"; }
  void Update(Boss& b) override {
    ++t_;
    b.FloatMove();
    b.charge = t_ < 30 ? static_cast<float>(t_) / 30.f : 0.f;
    if (t_ >= 30 && t_ < 130 && t_ % 8 == 0) {
      const float px = b.PlayerCenter().x;
      const float x = n_ % 2 ? RandRange(60.f, kW - 60.f) : Clamp(px + RandRange(-110.f, 110.f), 50.f, kW - 50.f);
      b.FireOrb({x, 20.f}, kPi / 2.f, 5.5f + static_cast<float>(b.Phase()) * 0.6f);
      ++n_;
    }
    if (t_ >= 160) b.ToFloat();
  }
};

// 그림자 기둥: 플레이어 주변 바닥에 마법진 예고 후 기둥 분출 (2단계부터 2연속)
class PillarState : public BossStateBase {
 public:
  const char* Name() const override { return "Pillar"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    const int ph = b.Phase();
    b.FloatMove();
    b.charge = t_ < 30 ? static_cast<float>(t_) / 30.f : 0.f;
    if (t_ == 30 || (ph >= 1 && t_ == 90)) {
      const int count = 2 + ph;
      const float px = b.PlayerCenter().x;
      for (int i = 0; i < count; ++i) {
        w.Spawn<PillarHazard>(Clamp(px + (static_cast<float>(i) - static_cast<float>(count - 1) / 2.f) * 95.f, 50.f, kW - 50.f));
      }
      w.Snd().Play(Sfx::PillarWarn);
    }
    if (t_ >= (ph >= 1 ? 170 : 110)) b.ToFloat();
  }
};

// 돌진: 반대편 벽으로 이동해 기를 모은 뒤 바닥을 가로질러 달린다. 끝난 뒤 빈틈(1.5배)
class DashState : public BossStateBase {
 public:
  const char* Name() const override { return "Dash"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    if (t_ == 1) {
      const float px = b.PlayerCenter().x;
      targetX_ = px < kW / 2.f ? kW - 100.f : 100.f;
      dashV_ = (targetX_ > kW / 2.f ? -1.f : 1.f) * (9.f + static_cast<float>(b.Phase()) * 1.5f);
    }
    if (sub_ == 0) {
      b.pos.x += Clamp((targetX_ - b.pos.x) * 0.08f, -8.f, 8.f);
      b.pos.y += (424.f - b.pos.y) * 0.1f;
      b.charge = std::min(1.f, static_cast<float>(t_) / 50.f);
      if ((t_ >= 55 && std::fabs(b.pos.x - targetX_) < 14.f) || t_ >= 110) {
        sub_ = 1; t_ = 0; b.charge = 0.f;
        w.Snd().Play(Sfx::BossDash);
      }
    } else if (sub_ == 1) {
      b.pos.x += dashV_;
      w.Fx().Shake(3.f);
      if (t_ % 2 == 0) w.Fx().Burst({b.pos.x, b.pos.y + 40.f}, 3, Hex(0xd9a0ff), 3.f, 16.f);
      if (b.pos.x <= 100.f || b.pos.x >= kW - 100.f) {
        b.pos.x = Clamp(b.pos.x, 100.f, kW - 100.f);
        sub_ = 2; t_ = 0;
        b.damageMultiplier = 1.5f;
        w.Fx().Shake(14.f);
        w.Snd().Play(Sfx::BossSlam);
      }
    } else if (t_ >= 45) {
      b.ToFloat();
    }
  }

 private:
  float targetX_ = 0.f, dashV_ = 0.f;
};

// 순간이동: 사라졌다가 반대편에 나타나 5갈래 탄을 두 번 쏜다 (사라지는 동안 무적)
class BlinkState : public BossStateBase {
 public:
  const char* Name() const override { return "Blink"; }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    const Vec2 pc = b.PlayerCenter();
    if (sub_ == 0) {
      b.invulnerable = true;
      b.alpha = std::max(0.f, b.alpha - 0.06f);
      if (b.alpha <= 0.f) {
        sub_ = 1; t_ = 0;
        b.pos.x = Clamp(kW - pc.x, 110.f, kW - 110.f);
        if (std::fabs(b.pos.x - pc.x) < 160.f) b.pos.x = pc.x < kW / 2.f ? kW - 110.f : 110.f;
        b.pos.y = 240.f;
        w.Fx().Burst(b.pos, 20, Hex(0xffb0ff), 5.f);
        w.Snd().Play(Sfx::BossBlink);
      }
    } else {
      b.alpha = std::min(1.f, b.alpha + 0.06f);
      if (b.alpha >= 1.f) b.invulnerable = false;
      b.charge = t_ < 30 ? static_cast<float>(t_) / 30.f : 0.f;
      if (t_ == 30 || t_ == 55) {
        const float base = std::atan2(pc.y - b.pos.y, pc.x - b.pos.x);
        for (int i = -2; i <= 2; ++i) b.FireOrb(b.pos, base + static_cast<float>(i) * 0.3f, 4.6f);
        w.Snd().Play(Sfx::BossShoot);
      }
      if (t_ >= 85) b.ToFloat();
    }
  }
};

// 사망: 연쇄 폭발 후 사라지며 스테이지 클리어(보스 격파)를 알린다
class DyingState : public BossStateBase {
 public:
  const char* Name() const override { return "Dying"; }
  void Enter(Boss& b) override {
    b.dying = true; b.invulnerable = true; b.contactEnabled = false; b.charge = 0.f; b.beam.active = false;
  }
  void Update(Boss& b) override {
    ++t_;
    World& w = b.GetWorld();
    w.Fx().Shake(6.f);
    b.pos.y += 0.3f;
    if (t_ % 6 == 0) {
      const Color colors[3] = {Hex(0xffb0ff), Hex(0xff6aa0), Hex(0xffffff)};
      w.Fx().Burst({b.pos.x + RandRange(-50.f, 50.f), b.pos.y + RandRange(-50.f, 50.f)}, 14, colors[(t_ / 6) % 3], 5.f, 40.f);
      w.Snd().Tone(RandRange(80.f, 200.f), RandRange(40.f, 80.f), 0.2f, Wave::Saw, 0.05f);
    }
    if (t_ > 140) {
      w.Fx().Burst(b.pos, 80, Hex(0xffd0ff), 9.f, 70.f);
      Session& s = w.Sess();
      s.score += 5000 + s.lives * 1000 + w.GetPlayer().Hp() * 200;
      w.NotifyBossDefeated();
      b.Kill();
    }
  }
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
  }
  return std::make_unique<FloatState>();
}

}  // namespace gengar

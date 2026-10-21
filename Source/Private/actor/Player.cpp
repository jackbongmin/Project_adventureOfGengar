#include "actor/Player.h"

#include <cmath>

#include "actor/PhantomArt.h"
#include "actor/boss/Boss.h"
#include "actor/enemy/Enemy.h"
#include "actor/projectile/ShadowBall.h"
#include "controller/PlayerController.h"
#include "core/Config.h"
#include "core/Session.h"
#include "data/PlayerStats.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

Player::Player(Vec2 p, int maxHp) : Character(ActorKind::Player, p, {22.f, 30.f}, maxHp) {
  SetController(std::make_unique<PlayerController>());
}

void Player::Update(World& w) {
  if (defeated_) { ++defeatedTimer_; return; }

  intent_ = Intent{};
  controller_->Think(*this, w, intent_);
  const Session& s = w.Sess();

  // ---- 수평 이동 (가속/마찰) ----
  const float maxSpeed = stats::MoveSpeed(s);
  if (intent_.moveX != 0.f) {
    vel.x += intent_.moveX * 0.55f;
    SetFacing(static_cast<int>(intent_.moveX));
  } else {
    vel.x *= onGround_ ? 0.78f : 0.93f;
  }
  vel.x = Clamp(vel.x, -maxSpeed, maxSpeed);
  if (intent_.moveX == 0.f && std::fabs(vel.x) < 0.05f) vel.x = 0.f;

  // ---- 점프 (코요테 타임 + 입력 버퍼 + 가변 높이) ----
  jumpBuffer_ = intent_.jumpPressed ? 7 : std::max(0, jumpBuffer_ - 1);
  coyote_ = onGround_ ? 6 : std::max(0, coyote_ - 1);
  if (jumpBuffer_ > 0 && coyote_ > 0) {
    vel.y = -stats::JumpSpeed(s);
    jumpBuffer_ = 0;
    coyote_ = 0;
    onGround_ = false;
    w.Snd().Play(Sfx::Jump);
  }
  if (!intent_.jumpHeld && vel.y < -3.5f) vel.y *= 0.8f;

  vel.y = std::min(vel.y + cfg::kGravity, cfg::kMaxFall);
  MoveAndCollide(w);
  if (pos.y > static_cast<float>(cfg::kRows * cfg::kTile) + 40.f) { Defeat(w); return; }

  // ---- 공격 ----
  if (cooldown_ > 0) --cooldown_;
  if (intent_.shoot && cooldown_ <= 0) Shoot(w);
  if (intent_.ultimate && s.gauge >= 100.f) CastUltimate(w);

  if (invuln_ > 0) --invuln_;
  ++animFrame_;
  CheckHazards(w);
}

void Player::Shoot(World& w) {
  const Session& s = w.Sess();
  const int n = stats::ShotCount(s);
  const float spreads[3][3] = {{0.f, 0.f, 0.f}, {-0.7f, 0.7f, 0.f}, {-1.5f, 0.f, 1.5f}};
  const Vec2 origin{pos.x + size.x * 0.5f + static_cast<float>(facing_) * 14.f, pos.y + 15.f};
  for (int i = 0; i < n; ++i) {
    w.Spawn<ShadowBall>(origin, Vec2{static_cast<float>(facing_) * 8.f, spreads[n - 1][i]}, stats::ShotRadius(s),
                        stats::Damage(s), stats::ShotLife(s));
  }
  cooldown_ = stats::FireCooldown(s);
  w.Snd().Play(Sfx::Shoot);
}

// 필살기: 화면 안의 모든 적에게 큰 피해, 적 탄환 제거, 잠시 무적
void Player::CastUltimate(World& w) {
  Session& s = w.Sess();
  s.gauge = 0.f;
  w.SetUltActive(true);
  const DamageInfo info{stats::UltDamage(s), 1, false, true};
  const Rect view{w.Cam().x - 40.f, 0.f, static_cast<float>(cfg::kScreenW) + 80.f, static_cast<float>(cfg::kScreenH)};
  w.ForEach(ActorKind::Enemy, [&](Actor& a) {
    if (a.Bounds().Overlaps(view)) static_cast<Enemy&>(a).TakeDamage(w, info);
  });
  if (Boss* boss = w.GetBoss()) boss->TakeDamage(w, DamageInfo{stats::UltBossDamage(s), 1, false, true});
  w.SetUltActive(false);

  w.ForEach(ActorKind::EnemyShot, [](Actor& a) { a.Kill(); });
  invuln_ = std::max(invuln_, 70);
  w.Fx().Shake(16.f);
  w.Fx().Flash();
  const Vec2 c = Bounds().Center();
  w.Fx().AddRing(c);
  w.Fx().Burst(c, 60, Hex(0xc58bff), 9.f, 40.f);
  w.Snd().Play(Sfx::Ultimate);
}

void Player::CheckHazards(World& w) {
  const Level& lv = w.GetLevel();
  const float T = static_cast<float>(cfg::kTile);
  const int c0 = static_cast<int>(std::floor((pos.x + 3.f) / T)), c1 = static_cast<int>(std::floor((pos.x + size.x - 3.f) / T));
  const int r = static_cast<int>(std::floor((pos.y + size.y - 1.f) / T));
  for (int c = c0; c <= c1; ++c) {
    if (lv.At(c, r) == Tile::Spike && pos.y + size.y > static_cast<float>(r) * T + 14.f) {
      TakeDamage(w, DamageInfo{1.f, -facing_});
      break;
    }
  }
  if (lv.HasFlag() && Bounds().Overlaps(lv.Flag()) && !w.StageCleared()) {
    Session& s = w.Sess();
    s.score += 500 + hp_ * 100;
    w.NotifyStageCleared();
    w.Snd().Play(Sfx::StageClear);
    w.Fx().Burst(Bounds().Center(), 40, Hex(0xffe27a), 6.f, 60.f);
  }
}

void Player::TakeDamage(World& w, const DamageInfo& info) {
  if (invuln_ > 0 || defeated_) return;
  --hp_;
  invuln_ = 100;
  vel.y = -6.f;
  vel.x = static_cast<float>(info.dir) * 5.f;
  w.Fx().Shake(8.f);
  w.Snd().Play(Sfx::PlayerHurt);
  w.Fx().Burst(Bounds().Center(), 12, Hex(0xc58bff), 4.f);
  if (hp_ <= 0) Defeat(w);
}

void Player::Defeat(World& w) {
  if (defeated_) return;
  defeated_ = true;
  defeatedTimer_ = 0;
  w.Fx().Shake(12.f);
  w.Fx().Burst(Bounds().Center(), 40, Hex(0xb36bff), 6.f, 50.f);
  w.Snd().Play(Sfx::PlayerDie);
}

void Player::Render(Graphics& gfx, const World&) {
  if (defeated_) return;
  if (invuln_ > 0 && (invuln_ / 4) % 2) return;  // 무적 깜빡임
  const Vec2 c = Bounds().Center();
  DrawPhantom(gfx, {c.x, c.y - 7.f}, facing_, static_cast<float>(animFrame_));  // 스프라이트 발끝을 히트박스 바닥에 맞춘다
}

}  // namespace gengar

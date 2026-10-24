#include "actor/enemy/Hopper.h"

#include <algorithm>
#include <cmath>

#include "actor/Player.h"
#include "actor/projectile/Shockwave.h"
#include "core/Config.h"
#include "core/Random.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {
namespace {

constexpr float kU = 2.f;          // 도트 한 칸 (px)
constexpr float kAggroRange = 420.f;

}  // namespace

Hopper::Hopper(Vec2 p, int stage) : Enemy(p, {28.f, 22.f}, 26 + 10 * stage), wait_(RandInt(30, 80)) {
  jumpSpeed_ = 10.5f + 0.5f * static_cast<float>(stage);
  facing_ = -1;
}

bool Hopper::SafeJump(const World& w, float vx, float vy) const {
  const float T = static_cast<float>(cfg::kTile);
  const float air = 2.f * vy / cfg::kGravity;  // 같은 높이에 떨어질 때까지 걸리는 프레임
  const float landX = pos.x + vx * air;
  const int feetRow = static_cast<int>(std::floor((pos.y + size.y + 2.f) / T));
  const Level& lv = w.GetLevel();
  for (float edge : {landX + 2.f, landX + size.x - 2.f}) {  // 몸 양끝이 모두 땅 위여야 한다
    const int col = static_cast<int>(std::floor(edge / T));
    bool ground = false;
    for (int r = feetRow; r < cfg::kRows && !ground; ++r) ground = lv.Solid(col, r);
    if (!ground) return false;
  }
  return true;
}

// 땅: 웅크리고 기다렸다가 점프. 공중: 뛰어오를 때 정한 가로 속도를 유지한다.
void Hopper::ApplyIntent(World& w) {
  if (!onGround_) { vel.x = jumpVx_; return; }
  if (air_) {  // 막 착지
    air_ = false;
    landed_ = true;
    wait_ = RandInt(45, 70) - (Elite() ? 10 : 0);
    w.Fx().Dust({pos.x + size.x * 0.5f, pos.y + size.y}, 5);
  }
  vel.x = 0.f;
  if (--wait_ == 20 && Elite()) warn_ = 20;  // 정예: 뛰기 직전에 "!"
  if (wait_ > 0) return;
  const Vec2 pc = w.GetPlayer().Bounds().Center(), me = Bounds().Center();
  const float dx = pc.x - me.x;
  float vy, vx;
  if (std::fabs(dx) < kAggroRange && std::fabs(pc.y - me.y) < 200.f) {
    // 플레이어가 있는 곳 근처에 떨어지도록 가로 속도를 정한다 (체공 약 40프레임)
    vy = jumpSpeed_ * (Elite() ? 1.15f : 1.f);
    vx = Clamp(dx / 40.f, -3.6f, 3.6f);
  } else {  // 멀면 바라보는 쪽으로 작게 뛴다
    if (HitWall()) facing_ = -facing_;
    vy = jumpSpeed_ * 0.6f;
    vx = static_cast<float>(facing_) * 1.4f;
  }
  // 떨어질 자리 아래가 구덩이면 가로 속도를 줄여 본다. 끝까지 안 되면 제자리에서 뛰고, 순찰 방향은 뒤집는다
  while (std::fabs(vx) > 0.01f && !SafeJump(w, vx, vy)) vx = std::fabs(vx) < 0.5f ? 0.f : vx - (vx > 0.f ? 0.5f : -0.5f);
  if (vx == 0.f && std::fabs(dx) >= kAggroRange) facing_ = -facing_;
  jumpVx_ = vx;
  vel.y = -vy;
  if (jumpVx_ != 0.f) SetFacing(jumpVx_ > 0.f ? 1 : -1);
  air_ = true;
  onGround_ = false;
}

// 정예: 착지하면 좌우로 짧은 충격파 (점프로 넘는다)
void Hopper::UpdateEliteSkill(World& w) {
  if (!landed_) return;
  landed_ = false;
  const float feetY = pos.y + size.y - 11.f, cx = pos.x + size.x * 0.5f;
  w.SpawnPooled<Shockwave>(Vec2{cx - 18.f, feetY}, -4.5f, 55);
  w.SpawnPooled<Shockwave>(Vec2{cx + 18.f, feetY}, 4.5f, 55);
  w.Fx().Shake(3.f);
  w.Snd().Play(Sfx::BossSlam);
}

// 도트 두꺼비 (2px 한 칸, 14x11칸). 웅크리면 몸이 낮아지고, 공중에서는 다리가 늘어진다.
void Hopper::Render(Graphics& g, const World&) {
  const bool crouch = onGround_ && wait_ < 16;
  const float bx = std::round(pos.x + size.x * 0.5f), by = std::round(pos.y + size.y);  // 발밑 가운데
  const bool hit = flash_ > 0;
  // 어두운 동굴에서도 보이도록 밝은 청록 몸 + 짙은 테두리
  const Color body = hit ? Hex(0xffffff) : Hex(0x36d1ac), dark = hit ? Hex(0xffffff) : Hex(0x0c2f2a);
  const Color belly = hit ? Hex(0xffffff) : Hex(0xc4fff0), eye = hit ? Hex(0xffffff) : Hex(0xff3f9a);
  const float f = static_cast<float>(facing_);
  // (col, row) 는 왼쪽 위 기준 칸. 바라보는 방향에 맞춰 좌우를 뒤집는다
  auto px = [&](float col, float row, float wcol, float hrow, Color c) {
    const float x0 = f > 0.f ? bx - 7.f * kU + col * kU : bx + 7.f * kU - (col + wcol) * kU;
    g.FillRect(x0, by - 11.f * kU + row * kU, wcol * kU, hrow * kU, c);
  };
  const float drop = crouch ? 2.f : 0.f;  // 웅크리면 몸을 2칸 내린다
  if (!onGround_) {  // 공중: 뒷다리가 아래로 쭉
    px(1.f, 9.f, 3.f, 2.f, dark);
    px(10.f, 9.f, 3.f, 2.f, dark);
  } else {
    px(0.f, 9.f, 4.f, 2.f, dark);
    px(10.f, 9.f, 4.f, 2.f, dark);
  }
  px(1.f, 2.f + drop, 12.f, 9.f - drop, dark);   // 테두리
  px(3.f, 0.f + drop, 8.f, 2.f, dark);
  px(2.f, 3.f + drop, 10.f, 7.f - drop, body);   // 몸통
  px(4.f, 1.f + drop, 6.f, 2.f, body);           // 머리 위 혹
  px(5.f, 6.f + drop, 5.f, 3.f - drop * 0.5f, belly);
  px(9.f, 3.f + drop, 2.f, 2.f, eye);            // 앞쪽 큰 눈
  px(9.f, 3.f + drop, 1.f, 1.f, hit ? Hex(0xffffff) : Hex(0xffe0f0));
  px(5.f, 3.f + drop, 2.f, 2.f, eye);
  px(11.f, 6.f + drop, 2.f, 1.f, dark);          // 입
}

}  // namespace gengar

#include "actor/pickup/Pickups.h"

#include <cmath>

#include "actor/PhantomArt.h"
#include "actor/Player.h"
#include "art/Art.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "data/PlayerStats.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

void Coin::Update(World& w) {
  Player& p = w.GetPlayer();
  if (!p.IsAlive()) return;
  const Vec2 pc = p.Bounds().Center();
  const float d = std::hypot(pos.x - pc.x, pos.y - pc.y);
  if (d >= 22.f && d < stats::MagnetRadius(w.Sess())) {  // 코인 자석: 범위 안의 코인이 날아온다
    pos += (pc - pos) * (7.f / d);
    return;
  }
  if (d < 22.f) {
    w.AddCoins(1);
    w.AddScore(10);
    w.Snd().Play(Sfx::Coin);
    w.Fx().Pop({pos.x, pos.y - 10.f}, "+1", Hex(0xffe27a));
    Kill();
  }
}

void Coin::Render(Graphics& g, const World& w) {
  const art::Assets& a = art::A();
  const SpriteId id = a.coin[(w.Frame() / 6 + static_cast<int>(pos.x / 32.f)) % 4];  // 회전하는 코인
  const float bob = std::round(std::sin(static_cast<float>(w.Frame()) * 0.05f + pos.x) * 1.5f) * 2.f;
  g.DrawSprite(id, pos.x - 8.f, pos.y - 8.f + bob, art::kPixel);
}

void Heart::Update(World& w) {
  Player& p = w.GetPlayer();
  if (!p.IsAlive()) return;
  const Vec2 pc = p.Bounds().Center();
  if (p.Hp() >= p.MaxHp()) {  // 풀피면 먹지 않고 그 자리에 남는다
    if (!skipLogged_ && std::hypot(pos.x - pc.x, pos.y - pc.y) < 24.f) { skipLogged_ = true; DebugLog("[heart] left in place (full hp %d/%d)", p.Hp(), p.MaxHp()); }
    return;
  }
  if (std::hypot(pos.x - pc.x, pos.y - pc.y) < 24.f) {
    DebugLog("[heart] collected (hp %d -> %d)", p.Hp(), p.Hp() + 1);
    p.Heal(1);
    w.Fx().Pop({pos.x, pos.y - 10.f}, "+HP", Hex(0xff7aa8));
    w.Snd().Play(Sfx::Heart);
    w.Fx().Burst(pos, 10, Hex(0xff7aa8), 3.f);
    Kill();
  }
}

void Heart::Render(Graphics& g, const World& w) {
  const float bob = std::round(std::sin(static_cast<float>(w.Frame()) * 0.08f + pos.x) * 1.5f) * 2.f;
  g.DrawSprite(art::A().heartFull, pos.x - 9.f, pos.y - 8.f + bob, art::kPixel);
}

void TreasureHat::Update(World& w) {
  constexpr float kHover = 30.f;  // 바닥에서 떠 있는 높이 (모자 중심 기준)
  if (!settled_) {
    // 아래쪽 첫 지면을 찾아 그 위로 깃털처럼 내려앉는다
    const int col = static_cast<int>(pos.x / cfg::kTile);
    float floorY = static_cast<float>(cfg::kGroundRow * cfg::kTile);
    for (int r = static_cast<int>((pos.y + 14.f) / cfg::kTile); r < cfg::kRows; ++r) {
      if (w.GetLevel().Solid(col, r)) { floorY = static_cast<float>(r * cfg::kTile); break; }
    }
    vel.y = std::min(vel.y + 0.04f, 1.4f);
    pos.y += vel.y;
    pos.x += std::sin(static_cast<float>(w.Frame()) * 0.07f) * 0.8f;
    if (pos.y >= floorY - kHover) { pos.y = floorY - kHover; settled_ = true; w.Fx().Burst(pos, 16, Hex(0xffe27a), 2.5f); }
    return;
  }
  ++settledFrames_;
  if (!w.BossDefeated()) return;  // 보스가 완전히 사라진 뒤에 주울 수 있다
  Player& p = w.GetPlayer();
  if (!p.IsAlive()) return;
  const Vec2 pc = p.Bounds().Center();
  const float dx = pc.x - pos.x, dy = pc.y - pos.y, dist = std::hypot(dx, dy);
  if (settledFrames_ > 300 && dist > 1.f) { pos.x += dx / dist * 2.5f; pos.y += dy / dist * 2.5f; }  // 줍지 않으면 끌려온다
  if (dist < 30.f) {
    DebugLog("[hat] collected -> ending");
    w.Fx().Burst(pos, 40, Hex(0xffe27a), 5.f, 50.f);
    w.Fx().Flash();
    w.NotifyHatCollected();
    Kill();
  }
}

void TreasureHat::Render(Graphics& g, const World& w) {
  const float t = static_cast<float>(w.Frame());
  const float bob = settled_ ? std::round(std::sin(t * 0.06f) * 2.f) * 2.f : 0.f;
  DrawTreasureHat(g, {pos.x, pos.y + 14.f + bob}, 1.f, 1.f, t);
  if (settled_ && w.BossDefeated() && (w.Frame() / 20) % 2 == 0) {  // 주우라는 도트 화살표
    const float ay = pos.y - 40.f + bob;
    g.FillRect(pos.x - 6.f, ay, 12, 4, Hex(0xffe27a));
    g.FillRect(pos.x - 4.f, ay + 4.f, 8, 4, Hex(0xffe27a));
    g.FillRect(pos.x - 2.f, ay + 8.f, 4, 4, Hex(0xffe27a));
  }
}

}  // namespace gengar

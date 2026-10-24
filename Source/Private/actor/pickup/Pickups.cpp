#include "actor/pickup/Pickups.h"

#include <algorithm>
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

void Chest::Update(World& w) {
  constexpr float kHalfH = 11.f;  // 상자 높이의 절반 (중심 -> 바닥)
  if (!landed_) {  // 아래쪽 첫 지면까지 떨어진다
    const int col = static_cast<int>(pos.x / cfg::kTile);
    float floorY = static_cast<float>(cfg::kGroundRow * cfg::kTile);
    for (int r = static_cast<int>((pos.y + kHalfH) / cfg::kTile); r < cfg::kRows; ++r) {
      if (w.GetLevel().Solid(col, r)) { floorY = static_cast<float>(r * cfg::kTile); break; }
    }
    vel.y = std::min(vel.y + cfg::kGravity, 9.f);
    pos.y += vel.y;
    if (pos.y + kHalfH >= floorY) {
      pos.y = floorY - kHalfH;
      landed_ = true;
      w.Fx().Dust({pos.x, floorY}, 6);
      w.Snd().Play(Sfx::CrumbleFall);
    }
    return;
  }
  if (opened_ >= 0) {
    if (++opened_ > 110) Kill();
    return;
  }
  Player& p = w.GetPlayer();
  if (!p.IsAlive() || !p.Bounds().Overlaps(Bounds())) return;
  opened_ = 0;
  DebugLog("[chest] opened at x=%d: +15 coins, bonus card", static_cast<int>(pos.x));
  w.AddCoins(15);
  w.AddScore(300);
  w.QueueCardPick();
  w.Fx().Burst(pos, 30, Hex(0xffe27a), 5.f, 30.f);
  w.Fx().Burst(pos, 14, Hex(0xc58bff), 3.f, 24.f);
  w.Fx().Pop({pos.x, pos.y - 30.f}, "BONUS CARD!", Hex(0x7ae0ff));
  w.Fx().Pop({pos.x, pos.y - 12.f}, "+15", Hex(0xffe27a));
  w.Fx().Shake(4.f);
  w.Snd().Play(Sfx::StageClear);
}

// 도트 상자: 나무 몸통 + 금테 + 보라 보석 자물쇠. 열리면 뚜껑이 들리고 빛이 솟는다.
void Chest::Render(Graphics& g, const World& w) {
  const float t = static_cast<float>(w.Frame());
  const float x = std::floor(pos.x - 16.f), y = std::floor(pos.y - 11.f);
  float alpha = 1.f;
  if (opened_ > 70) alpha = 1.f - static_cast<float>(opened_ - 70) / 40.f;
  auto C = [&](Color c) { return WithAlpha(c, alpha); };
  if (opened_ < 0) {
    g.Glow(pos.x, pos.y, 34.f + std::sin(t * 0.1f) * 4.f, Rgb(255, 210, 90, 60));
    if ((w.Frame() / 20) % 2 == 0) {  // 머리 위에서 반짝이는 별
      g.FillRect(pos.x - 1.f, y - 16.f, 2.f, 6.f, Hex(0xffe27a));
      g.FillRect(pos.x - 3.f, y - 14.f, 6.f, 2.f, Hex(0xffe27a));
    }
  } else {
    const float k = std::min(1.f, static_cast<float>(opened_) / 12.f);
    g.Glow(pos.x, y - 10.f, 30.f + 30.f * k, Rgb(255, 230, 140, static_cast<int>(120.f * alpha)));
    g.FillRect(pos.x - 6.f, y - 60.f * k, 12.f, 60.f * k, Rgb(255, 240, 180, static_cast<int>(90.f * alpha)));
  }
  // 몸통
  g.FillRect(x, y + 8.f, 32.f, 14.f, C(Hex(0x6b3a1f)));
  g.FillRect(x + 2.f, y + 10.f, 28.f, 10.f, C(Hex(0x8a4f2a)));
  g.FillRect(x, y + 8.f, 32.f, 2.f, C(Hex(0xffc93c)));
  g.FillRect(x + 6.f, y + 8.f, 4.f, 14.f, C(Hex(0xffc93c)));
  g.FillRect(x + 22.f, y + 8.f, 4.f, 14.f, C(Hex(0xffc93c)));
  // 뚜껑 (열리면 위로 들린다)
  const float lid = opened_ < 0 ? 0.f : -std::min(8.f, static_cast<float>(opened_));
  g.FillRect(x, y + lid, 32.f, 8.f, C(Hex(0x5a2f18)));
  g.FillRect(x + 2.f, y + 2.f + lid, 28.f, 4.f, C(Hex(0x7a4424)));
  g.FillRect(x, y + lid, 32.f, 2.f, C(Hex(0xffc93c)));
  // 자물쇠 보석
  g.FillRect(x + 13.f, y + 6.f, 6.f, 8.f, C(Hex(0xffc93c)));
  g.FillRect(x + 14.f, y + 8.f, 4.f, 4.f, C(opened_ < 0 && (w.Frame() / 15) % 2 ? Hex(0xffffff) : Hex(0xc58bff)));
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

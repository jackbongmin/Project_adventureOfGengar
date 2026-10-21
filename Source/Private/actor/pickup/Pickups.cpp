#include "actor/pickup/Pickups.h"

#include <cmath>

#include "actor/Player.h"
#include "art/Art.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

void Coin::Update(World& w) {
  Player& p = w.GetPlayer();
  if (!p.IsAlive()) return;
  const Vec2 pc = p.Bounds().Center();
  if (std::hypot(pos.x - pc.x, pos.y - pc.y) < 22.f) {
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
  if (!p.IsAlive() || p.Hp() >= p.MaxHp()) return;  // 풀피면 먹지 않는다
  const Vec2 pc = p.Bounds().Center();
  if (std::hypot(pos.x - pc.x, pos.y - pc.y) < 24.f) {
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

}  // namespace gengar

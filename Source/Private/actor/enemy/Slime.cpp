#include "actor/enemy/Slime.h"

#include <cmath>

#include "controller/PatrolController.h"
#include "art/Art.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

Slime::Slime(Vec2 p, int stage) : Enemy(p, {28.f, 22.f}, 2 + stage) {
  moveSpeed_ = 0.8f + 0.2f * static_cast<float>(stage);
  SetController(std::make_unique<PatrolController>(-1));
  facing_ = -1;
}

void Slime::Render(Graphics& g, const World& w) {
  const art::Assets& a = art::A();
  const SpriteId id = a.slime[(w.Frame() / 14) % 2];
  const Vec2 sz = g.SpriteSize(id);
  const float x = pos.x + size.x * 0.5f - sz.x * art::kPixel * 0.5f, y = pos.y + size.y - sz.y * art::kPixel;
  g.DrawSprite(id, x, y, art::kPixel);
  if (flash_ > 0) g.DrawSprite(id, x, y, art::kPixel, false, 1.f, true);
}

}  // namespace gengar

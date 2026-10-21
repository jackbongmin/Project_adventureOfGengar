#include "actor/enemy/Hound.h"

#include <cmath>

#include "controller/ChaseController.h"
#include "art/Art.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

Hound::Hound(Vec2 p, int stage) : Enemy(p, {34.f, 24.f}, 3 + stage) {
  moveSpeed_ = 2.1f + 0.25f * static_cast<float>(stage);  // 추적 속도
  jumpSpeed_ = 9.5f;
  SetController(std::make_unique<ChaseController>(0.7f / moveSpeed_));  // 순찰 속도 0.7
  facing_ = -1;
}

void Hound::Render(Graphics& g, const World& w) {
  const art::Assets& a = art::A();
  const int frame = (w.Frame() / (alerted_ ? 4 : 8)) % 2;  // 추적 중에는 빠르게 달린다
  const SpriteId id = alerted_ ? a.houndAlert[frame] : a.houndCalm[frame];
  const Vec2 sz = g.SpriteSize(id), c = Bounds().Center();
  const float x = c.x - sz.x * art::kPixel * 0.5f, y = pos.y + size.y - sz.y * art::kPixel;
  g.DrawSprite(id, x, y, art::kPixel, facing_ < 0);
  if (flash_ > 0) g.DrawSprite(id, x, y, art::kPixel, facing_ < 0, 1.f, true);
}

}  // namespace gengar

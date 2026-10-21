#include "actor/enemy/Bat.h"

#include <cmath>

#include "controller/FlyController.h"
#include "core/Random.h"
#include "art/Art.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

Bat::Bat(Vec2 p, int stage, float range) : Enemy(p, {26.f, 20.f}, 1 + stage), flapPhase_(RandRange(0.f, 6.f)) {
  SetController(std::make_unique<FlyController>(range, 0.f));
}

void Bat::ApplyIntent(World&) { vel = intent_.flyVel; }

void Bat::StepPhysics(World&) { pos += vel; }

void Bat::Render(Graphics& g, const World& w) {
  const art::Assets& a = art::A();
  const SpriteId id = a.bat[(w.Frame() / 5 + static_cast<int>(flapPhase_)) % 2];
  const Vec2 sz = g.SpriteSize(id), c = Bounds().Center();
  const float x = c.x - sz.x * art::kPixel * 0.5f, y = c.y - sz.y * art::kPixel * 0.5f;
  g.DrawSprite(id, x, y, art::kPixel);
  if (flash_ > 0) g.DrawSprite(id, x, y, art::kPixel, false, 1.f, true);
}

}  // namespace gengar

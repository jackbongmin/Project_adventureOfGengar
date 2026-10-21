#include "actor/Character.h"

#include <cmath>

#include "core/Config.h"
#include "world/World.h"

namespace gengar {

Character::Character(ActorKind kind, Vec2 p, Vec2 s, int maxHp) : Actor(kind), hp_(maxHp), maxHp_(maxHp) {
  pos = p;
  size = s;
}

void Character::MoveAndCollide(const World& world) {
  const Level& lv = world.GetLevel();
  const float T = static_cast<float>(cfg::kTile);

  // ---- X축 ----
  hitWall_ = false;
  pos.x += vel.x;
  {
    const int r0 = static_cast<int>(std::floor(pos.y / T));
    const int r1 = static_cast<int>(std::floor((pos.y + size.y - 0.01f) / T));
    if (vel.x > 0.f) {
      const int c = static_cast<int>(std::floor((pos.x + size.x - 0.01f) / T));
      for (int r = r0; r <= r1; ++r) {
        if (lv.Solid(c, r)) { pos.x = static_cast<float>(c) * T - size.x; vel.x = 0.f; hitWall_ = true; break; }
      }
    } else if (vel.x < 0.f) {
      const int c = static_cast<int>(std::floor(pos.x / T));
      for (int r = r0; r <= r1; ++r) {
        if (lv.Solid(c, r)) { pos.x = static_cast<float>(c + 1) * T; vel.x = 0.f; hitWall_ = true; break; }
      }
    }
  }

  // ---- Y축 ----
  onGround_ = false;
  pos.y += vel.y;
  {
    const int c0 = static_cast<int>(std::floor(pos.x / T));
    const int c1 = static_cast<int>(std::floor((pos.x + size.x - 0.01f) / T));
    if (vel.y > 0.f) {
      const int r = static_cast<int>(std::floor((pos.y + size.y - 0.01f) / T));
      for (int c = c0; c <= c1; ++c) {
        if (lv.Solid(c, r)) { pos.y = static_cast<float>(r) * T - size.y; vel.y = 0.f; onGround_ = true; break; }
      }
    } else if (vel.y < 0.f) {
      const int r = static_cast<int>(std::floor(pos.y / T));
      for (int c = c0; c <= c1; ++c) {
        if (lv.Solid(c, r)) { pos.y = static_cast<float>(r + 1) * T; vel.y = 0.f; break; }
      }
    }
  }
}

bool Character::AheadBlocked(const World& world, int dir) const {
  const Level& lv = world.GetLevel();
  const float T = static_cast<float>(cfg::kTile);
  const int col = static_cast<int>(std::floor((dir > 0 ? pos.x + size.x + 2.f : pos.x - 2.f) / T));
  if (lv.At(col, static_cast<int>(std::floor((pos.y + size.y - 2.f) / T))) == Tile::Spike) return true;
  return onGround_ && !lv.Solid(col, static_cast<int>(std::floor((pos.y + size.y + 4.f) / T)));
}

}  // namespace gengar

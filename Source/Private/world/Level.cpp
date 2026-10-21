#include "world/Level.h"

#include "actor/Player.h"
#include "platform/Audio.h"
#include "world/Effects.h"
#include "world/World.h"

namespace gengar {

Level::Level(int widthTiles, int theme, std::string name)
    : width_(widthTiles),
      theme_(theme),
      name_(std::move(name)),
      tiles_(static_cast<size_t>(widthTiles * cfg::kRows), Tile::Empty) {
  // 기본 지형: 바닥 2줄
  for (int r = cfg::kGroundRow; r < cfg::kRows; ++r) {
    for (int c = 0; c < width_; ++c) Set(c, r, Tile::Ground);
  }
}

Tile Level::At(int c, int r) const {
  if (c < 0 || c >= width_) return Tile::Ground;
  if (r < 0 || r >= cfg::kRows) return Tile::Empty;
  return tiles_[static_cast<size_t>(r * width_ + c)];
}

bool Level::Solid(int c, int r) const {
  const Tile t = At(c, r);
  return t == Tile::Ground || t == Tile::Brick || t == Tile::Crumble;
}

void Level::Set(int c, int r, Tile t) {
  if (c < 0 || c >= width_ || r < 0 || r >= cfg::kRows) return;
  tiles_[static_cast<size_t>(r * width_ + c)] = t;
}

void Level::AddCrumble(int c0, int c1, int row) {
  crumbles_.emplace_back(c0, c1, row);
  const size_t idx = crumbles_.size() - 1;
  for (int c = c0; c <= c1; ++c) {
    Set(c, row, Tile::Crumble);
    crumbIndex_[row * 1000 + c] = idx;
  }
}

const CrumbleGroup* Level::CrumbleAt(int c, int r) const {
  const auto it = crumbIndex_.find(r * 1000 + c);
  return it == crumbIndex_.end() ? nullptr : &crumbles_[it->second];
}

void Level::UpdateCrumbles(World& world) {
  Player& p = world.GetPlayer();
  const float T = static_cast<float>(cfg::kTile);
  for (CrumbleGroup& g : crumbles_) {
    const float x0 = static_cast<float>(g.c0) * T, x1 = static_cast<float>(g.c1 + 1) * T;
    const float top = static_cast<float>(g.row) * T;
    const Rect pb = p.Bounds();
    switch (g.state) {
      case CrumbleGroup::State::Idle:
        if (p.IsAlive() && p.OnGround() && std::fabs(pb.Bottom() - top) < 1.5f && pb.Right() > x0 && pb.x < x1) {
          g.state = CrumbleGroup::State::Shake;
          g.timer = 0;
          world.Snd().Play(Sfx::CrumbleShake);
        }
        break;
      case CrumbleGroup::State::Shake:
        if (++g.timer % 12 == 0) world.Snd().Play(Sfx::CrumbleShake);
        if (g.timer >= cfg::kCrumbleFrames) {
          g.state = CrumbleGroup::State::Gone;
          g.timer = 0;
          for (int c = g.c0; c <= g.c1; ++c) {
            Set(c, g.row, Tile::Empty);
            world.Fx().AddChunk({static_cast<float>(c) * T, top});
            world.Fx().Burst({static_cast<float>(c) * T + 16.f, top + 16.f}, 6, Hex(0xb58a5a), 3.f, 22.f);
          }
          world.Snd().Play(Sfx::CrumbleFall);
        }
        break;
      case CrumbleGroup::State::Gone:
        if (++g.timer >= cfg::kCrumbleRespawn) {
          const bool blocked = p.IsAlive() && pb.Right() > x0 && pb.x < x1 && pb.Bottom() > top && pb.y < top + T;
          if (!blocked) {
            g.state = CrumbleGroup::State::Idle;
            g.timer = 0;
            for (int c = g.c0; c <= g.c1; ++c) Set(c, g.row, Tile::Crumble);
          }
        }
        break;
    }
  }
}

}  // namespace gengar

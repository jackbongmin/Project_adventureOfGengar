#include "world/LevelRenderer.h"

#include <cmath>

#include "art/Art.h"
#include "core/Config.h"
#include "core/Random.h"
#include "platform/Graphics.h"

namespace gengar {
namespace {

constexpr float W = static_cast<float>(cfg::kScreenW);
constexpr float H = static_cast<float>(cfg::kScreenH);
constexpr float T = static_cast<float>(cfg::kTile);

struct Theme {
  Color sky[3];
  Color hill[2];
};

const Theme& GetTheme(int index) {
  static const Theme themes[3] = {
      {{Hex(0x2b1850), Hex(0x6d4a94), Hex(0xc18bb8)}, {Hex(0x3a2466), Hex(0x2c1a52)}},
      {{Hex(0x070914), Hex(0x10142b), Hex(0x1d2347)}, {Hex(0x161b3a), Hex(0x0f1330)}},
      {{Hex(0x12010f), Hex(0x34082b), Hex(0x5a0f3d)}, {Hex(0x2a0a2a), Hex(0x1c061d)}},
  };
  return themes[Clamp(index, 0, 2)];
}

// 하늘은 부드러운 그라데이션 대신 16px 띠로 끊어서 도트 느낌을 낸다.
Color SkyAt(const Theme& th, float t) {
  t = std::floor(t * 24.f) / 24.f;
  if (t < 0.6f) return Lerp(th.sky[0], th.sky[1], t / 0.6f);
  return Lerp(th.sky[1], th.sky[2], (t - 0.6f) / 0.4f);
}

}  // namespace

void RenderLevelBackground(Graphics& gfx, const Level& level, float camX, int frame) {
  const art::Assets& assets = art::A();
  const Theme& th = GetTheme(level.Theme());
  const int theme = level.Theme();

  for (float y = 0; y < H; y += 16.f) gfx.FillRect(0, y, W, 16.f, SkyAt(th, y / H));

  // 별 (2x2 도트, 일부는 깜빡인다)
  const float starAlpha = theme == 1 ? 0.45f : 0.9f;
  for (int i = 0; i < 70; ++i) {
    float x = std::fmod(static_cast<float>(i) * 137.f - camX * 0.08f, W);
    if (x < 0) x += W;
    x = std::floor(x / 2.f) * 2.f;
    const float y = static_cast<float>((i * 53) % 280);
    const bool on = std::sin(static_cast<float>(frame) * 0.05f + static_cast<float>(i)) > -0.4f;
    if (on) gfx.FillRect(x, y, i % 5 == 0 ? 4.f : 2.f, i % 5 == 0 ? 4.f : 2.f, Rgb(255, 255, 255, static_cast<int>(starAlpha * 255)));
  }

  if (theme == 0) {  // 달
    const Vec2 ms = gfx.SpriteSize(assets.moon);
    gfx.DrawSprite(assets.moon, std::floor(780.f - camX * 0.03f - ms.x * 2.f), 110.f - ms.y * 2.f, 4);
  }
  if (theme == 2) {  // 회전하는 마법진 (도트 링 + 도트 선)
    const float rot = static_cast<float>(frame) * 0.004f;
    const Color c = Rgb(255, 80, 200, 90);
    gfx.PixelRing(W * 0.5f, 250.f, 200.f, 4.f, c, 4.f);
    gfx.PixelRing(W * 0.5f, 250.f, 150.f, 4.f, c, 4.f);
    for (int i = 0; i < 6; ++i) {
      const float a0 = rot + static_cast<float>(i) * kPi / 3.f, a1 = a0 + 2.094f;
      gfx.PixelLine({W * 0.5f + std::cos(a0) * 200.f, 250.f + std::sin(a0) * 200.f},
                    {W * 0.5f + std::cos(a1) * 200.f, 250.f + std::sin(a1) * 200.f}, 4.f, c, 4.f);
    }
  }

  // 시차 스크롤 언덕/수정: 8px 폭의 계단식 기둥
  for (int layer = 0; layer < 2; ++layer) {
    const float f = 0.2f + static_cast<float>(layer) * 0.25f, base = 380.f + static_cast<float>(layer) * 40.f;
    for (float x = 0; x < W; x += 8.f) {
      const float wx = std::floor((x + camX * f) / 8.f) * 8.f;
      float y = base + std::sin(wx * 0.008f + static_cast<float>(layer)) * 32.f + std::sin(wx * 0.021f) * 14.f;
      if (theme == 1) y = base - std::fabs(std::sin(wx * 0.012f + static_cast<float>(layer) * 2.f)) * 90.f + 20.f;
      y = std::floor(y / 8.f) * 8.f;
      gfx.FillRect(x, y, 8.f, H - y, th.hill[layer]);
    }
  }
  if (theme == 1) {  // 종유석: 폭이 줄어드는 도트 계단
    for (int i = 0; i < 22; ++i) {
      float x = std::fmod(static_cast<float>(i) * 90.f - camX * 0.9f, W + 90.f);
      if (x < 0) x += W + 90.f;
      x = std::floor((x - 45.f) / 4.f) * 4.f;
      const int rows = (40 + (i * 37) % 60) / 4;
      for (int k = 0; k < rows; ++k) {
        const float w = std::max(4.f, std::floor((1.f - static_cast<float>(k) / static_cast<float>(rows)) * 10.f) * 4.f);
        gfx.FillRect(x - w * 0.5f, 60.f + static_cast<float>(k) * 4.f, w, 4.f, Hex(0x0b0e1f));
      }
    }
  }
}

void RenderLevelTiles(Graphics& gfx, const Level& level, float camX, int frame) {
  const art::TileSet& ts = art::A().tiles[Clamp(level.Theme(), 0, 2)];
  const int c0 = std::max(0, static_cast<int>(std::floor(camX / T)));
  const int c1 = std::min(level.WidthTiles() - 1, static_cast<int>(std::floor((camX + W) / T)));

  for (int c = c0; c <= c1; ++c) {
    for (int r = 0; r < cfg::kRows; ++r) {
      const Tile tile = level.At(c, r);
      if (tile == Tile::Empty) continue;
      const float x = static_cast<float>(c) * T, y = static_cast<float>(r) * T;
      switch (tile) {
        case Tile::Ground: gfx.DrawSprite(level.Solid(c, r - 1) ? ts.dirt : ts.groundTop, x, y, art::kPixel); break;
        case Tile::Brick: gfx.DrawSprite(ts.brick, x, y, art::kPixel); break;
        case Tile::Spike: gfx.DrawSprite(ts.spike, x, y, art::kPixel); break;
        case Tile::Crumble: {
          const CrumbleGroup* g = level.CrumbleAt(c, r);
          const bool shaking = g && g->state == CrumbleGroup::State::Shake;
          const float ox = shaking ? static_cast<float>(RandInt(-1, 1)) * 2.f : 0.f;  // 흔들림도 2px(도트 1개) 단위
          const bool hot = shaking && (g->timer / 5) % 2 == 1;
          gfx.DrawSprite(hot ? ts.crumbleHot : ts.crumble, x + ox, y, art::kPixel);
          break;
        }
        case Tile::Empty: break;
      }
    }
  }

  // 깃발: 장대 + 물결치는 도트 깃발
  if (level.HasFlag()) {
    const Rect& f = level.Flag();
    const float x = f.x + 6.f, y = f.y;
    gfx.FillRect(x, y, 4, f.h, Hex(0xd8d0f0));
    gfx.FillRect(x + 1, y, 2, f.h, Hex(0xffffff));
    gfx.FillRect(x - 2, y - 6, 8, 8, Hex(0xffe27a));
    gfx.DrawSprite(art::A().pennant[(frame / 8) % 3], x + 4.f, y + 6.f, art::kPixel);
  }
}

}  // namespace gengar

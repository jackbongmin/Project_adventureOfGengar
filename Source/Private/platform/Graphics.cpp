#include "platform/Graphics.h"

#include <SDL.h>

#include <algorithm>
#include <cmath>

#include "platform/PixelFont.h"

namespace gengar {

Graphics::~Graphics() { Shutdown(); }

bool Graphics::Init(const char* title, int w, int h) {
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");  // nearest-neighbor 확대
  window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h,
                             SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!window_) return false;
  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer_) renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
  if (!renderer_) return false;
  SDL_RenderSetLogicalSize(renderer_, w, h);
  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
  return true;
}

void Graphics::Shutdown() {
  for (SpriteTex& s : sprites_) {
    if (s.tex) SDL_DestroyTexture(s.tex);
    if (s.flash) SDL_DestroyTexture(s.flash);
  }
  sprites_.clear();
  if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
  if (window_) { SDL_DestroyWindow(window_); window_ = nullptr; }
}

void Graphics::Begin(Color c) {
  SDL_SetRenderDrawColor(renderer_, c.r, c.g, c.b, 255);
  SDL_RenderClear(renderer_);
  ox_ = oy_ = 0.f;
  galpha_ = 1.f;
  xf_.clear();
}

void Graphics::End() { SDL_RenderPresent(renderer_); }

void Graphics::PushTransform(Vec2 origin, float scale, int flipX) { xf_.push_back({origin, scale, flipX}); }
void Graphics::PopTransform() { if (!xf_.empty()) xf_.pop_back(); }

Vec2 Graphics::Map(Vec2 p) const {
  if (xf_.empty()) return {p.x - ox_, p.y - oy_};
  const Xf& t = xf_.back();
  return {t.origin.x + p.x * t.scale * static_cast<float>(t.flip) - ox_, t.origin.y + p.y * t.scale - oy_};
}

void Graphics::Tris(const std::vector<Vec2>& pts, Color c) {
  c = Apply(c);
  if (c.a == 0 || pts.empty()) return;
  static std::vector<SDL_Vertex> verts;
  verts.resize(pts.size());
  for (size_t i = 0; i < pts.size(); ++i) {
    verts[i].position = {pts[i].x, pts[i].y};
    verts[i].color = {c.r, c.g, c.b, c.a};
    verts[i].tex_coord = {0.f, 0.f};
  }
  SDL_RenderGeometry(renderer_, nullptr, verts.data(), static_cast<int>(verts.size()), nullptr, 0);
}

void Graphics::FillRect(float x, float y, float w, float h, Color c) {
  if (xf_.empty()) {
    c = Apply(c);
    if (c.a == 0) return;
    const float x0 = std::round(x - ox_), y0 = std::round(y - oy_);
    const float x1 = std::round(x - ox_ + w), y1 = std::round(y - oy_ + h);
    SDL_FRect r{x0, y0, x1 - x0, y1 - y0};
    SDL_SetRenderDrawColor(renderer_, c.r, c.g, c.b, c.a);
    SDL_RenderFillRectF(renderer_, &r);
  } else {
    FillQuad({x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}, c);
  }
}

void Graphics::FillTriangle(Vec2 a, Vec2 b, Vec2 c, Color col) {
  scratch_.assign({Map(a), Map(b), Map(c)});
  Tris(scratch_, col);
}

void Graphics::FillQuad(Vec2 a, Vec2 b, Vec2 c, Vec2 d, Color col) {
  const Vec2 ma = Map(a), mb = Map(b), mc = Map(c), md = Map(d);
  scratch_.assign({ma, mb, mc, ma, mc, md});
  Tris(scratch_, col);
}

void Graphics::FillEllipse(float cx, float cy, float rx, float ry, Color col, float rot) {
  const int seg = Clamp(static_cast<int>(std::max(rx, ry) * Scale() * 0.9f), 10, 48);
  const float cr = std::cos(rot), sr = std::sin(rot);
  const Vec2 center = Map({cx, cy});
  auto P = [&](float a) {
    const float lx = rx * std::cos(a), ly = ry * std::sin(a);
    return Map({cx + lx * cr - ly * sr, cy + lx * sr + ly * cr});
  };
  scratch_.clear();
  Vec2 prev = P(0.f);
  for (int i = 1; i <= seg; ++i) {
    const Vec2 cur = P(2.f * kPi * static_cast<float>(i) / static_cast<float>(seg));
    scratch_.push_back(center);
    scratch_.push_back(prev);
    scratch_.push_back(cur);
    prev = cur;
  }
  Tris(scratch_, col);
}

void Graphics::Line(Vec2 a, Vec2 b, float thickness, Color c) {
  const Vec2 ma = Map(a), mb = Map(b);
  const float dx = mb.x - ma.x, dy = mb.y - ma.y, len = std::sqrt(dx * dx + dy * dy);
  if (len < 1e-3f) return;
  const float h = thickness * Scale() * 0.5f;
  const Vec2 n{-dy / len * h, dx / len * h};
  const Vec2 p0 = ma + n, p1 = mb + n, p2 = mb - n, p3 = ma - n;
  scratch_.assign({p0, p1, p2, p0, p2, p3});
  Tris(scratch_, c);
}

void Graphics::DashedLine(Vec2 a, Vec2 b, float thickness, float dash, float gap, Color c) {  // 도트 느낌의 점선
  const float dx = b.x - a.x, dy = b.y - a.y, len = std::sqrt(dx * dx + dy * dy);
  if (len < 1e-3f) return;
  const Vec2 dir{dx / len, dy / len};
  for (float d = 0.f; d < len; d += dash + gap) {
    const float e = std::min(d + dash, len);
    PixelLine(a + dir * d, a + dir * e, thickness, c);
  }
}

void Graphics::Ring(float cx, float cy, float r, float thickness, Color col) {
  const int seg = Clamp(static_cast<int>(r * Scale() * 0.6f), 16, 96);
  const float ro = r + thickness * 0.5f, ri = std::max(0.f, r - thickness * 0.5f);
  scratch_.clear();
  auto P = [&](float rad, float a) { return Map({cx + std::cos(a) * rad, cy + std::sin(a) * rad}); };
  for (int i = 0; i < seg; ++i) {
    const float a0 = 2.f * kPi * static_cast<float>(i) / static_cast<float>(seg);
    const float a1 = 2.f * kPi * static_cast<float>(i + 1) / static_cast<float>(seg);
    const Vec2 o0 = P(ro, a0), o1 = P(ro, a1), i0 = P(ri, a0), i1 = P(ri, a1);
    scratch_.insert(scratch_.end(), {o0, o1, i1, o0, i1, i0});
  }
  Tris(scratch_, col);
}

void Graphics::Glow(float cx, float cy, float r, Color c) {
  constexpr int kLayers = 7;
  Color layer = c;
  layer.a = static_cast<uint8_t>(std::min(255.f, static_cast<float>(c.a) / (kLayers * 0.55f)));
  for (int i = 0; i < kLayers; ++i) {
    const float k = static_cast<float>(kLayers - i) / static_cast<float>(kLayers);
    FillCircle(cx, cy, r * k, layer);
  }
}

void Graphics::Heart(float x, float y, float s, bool full) {
  const Color fill = full ? Hex(0xff4f86) : Rgb(30, 10, 40, 180);
  const Color edge = full ? Hex(0xffd0e0) : Hex(0x6b4a8a);
  auto draw = [&](float grow, Color c) {
    FillCircle(x - 4.6f * s, y - 4.8f * s, (5.2f + grow) * s, c);
    FillCircle(x + 4.6f * s, y - 4.8f * s, (5.2f + grow) * s, c);
    FillTriangle({x - (9.6f + grow) * s, y - 2.4f * s}, {x + (9.6f + grow) * s, y - 2.4f * s},
                 {x, y + (6.5f + grow) * s}, c);
  };
  draw(1.2f, edge);
  draw(0.f, fill);
}

SpriteId Graphics::CreateSprite(const std::vector<std::string>& rows, const std::vector<PaletteEntry>& palette) {
  const int h = static_cast<int>(rows.size());
  const int w = h > 0 ? static_cast<int>(rows[0].size()) : 0;
  if (w == 0 || h == 0) return kNoSprite;

  std::vector<uint8_t> color(static_cast<size_t>(w * h * 4), 0), white(static_cast<size_t>(w * h * 4), 0);
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      const char ch = x < static_cast<int>(rows[static_cast<size_t>(y)].size()) ? rows[static_cast<size_t>(y)][static_cast<size_t>(x)] : '.';
      if (ch == '.') continue;
      Color col = Hex(0xff00ff);  // 팔레트에 없는 문자는 눈에 띄는 분홍색
      for (const PaletteEntry& e : palette) {
        if (e.key == ch) { col = e.color; break; }
      }
      const size_t i = static_cast<size_t>((y * w + x) * 4);
      color[i] = col.r; color[i + 1] = col.g; color[i + 2] = col.b; color[i + 3] = col.a;
      white[i] = white[i + 1] = white[i + 2] = 255; white[i + 3] = col.a;
    }
  }
  auto make = [&](const std::vector<uint8_t>& px) {
    SDL_Texture* t = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, w, h);
    if (!t) return t;
    SDL_UpdateTexture(t, nullptr, px.data(), w * 4);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(t, SDL_ScaleModeNearest);
    return t;
  };
  SpriteTex s;
  s.tex = make(color);
  s.flash = make(white);
  s.w = w;
  s.h = h;
  sprites_.push_back(s);
  return static_cast<SpriteId>(sprites_.size() - 1);
}

Vec2 Graphics::SpriteSize(SpriteId id) const {
  if (id < 0 || id >= static_cast<SpriteId>(sprites_.size())) return {0.f, 0.f};
  return {static_cast<float>(sprites_[static_cast<size_t>(id)].w), static_cast<float>(sprites_[static_cast<size_t>(id)].h)};
}

void Graphics::DrawSprite(SpriteId id, float x, float y, int scale, bool flipX, float alpha, bool flash) {
  if (id < 0 || id >= static_cast<SpriteId>(sprites_.size())) return;
  const SpriteTex& s = sprites_[static_cast<size_t>(id)];
  SDL_Texture* t = flash ? s.flash : s.tex;
  if (!t) return;
  SDL_SetTextureAlphaMod(t, static_cast<Uint8>(Clamp(alpha * galpha_, 0.f, 1.f) * 255.f));
  const SDL_FRect dst{std::floor(x - ox_), std::floor(y - oy_), static_cast<float>(s.w * scale), static_cast<float>(s.h * scale)};
  SDL_RenderCopyExF(renderer_, t, nullptr, &dst, 0.0, nullptr, flipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void Graphics::DrawSpriteSized(SpriteId id, float x, float y, float w, float h, bool flipX, float alpha) {
  if (id < 0 || id >= static_cast<SpriteId>(sprites_.size())) return;
  SDL_Texture* t = sprites_[static_cast<size_t>(id)].tex;
  if (!t) return;
  SDL_SetTextureAlphaMod(t, static_cast<Uint8>(Clamp(alpha * galpha_, 0.f, 1.f) * 255.f));
  const SDL_FRect dst{std::floor(x - ox_), std::floor(y - oy_), std::round(w), std::round(h)};
  SDL_RenderCopyExF(renderer_, t, nullptr, &dst, 0.0, nullptr, flipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void Graphics::PixelLine(Vec2 a, Vec2 b, float thickness, Color c, float grid) {
  const float dx = b.x - a.x, dy = b.y - a.y, len = std::sqrt(dx * dx + dy * dy);
  const int steps = std::max(1, static_cast<int>(len / grid));
  const float t = std::max(grid, thickness);
  for (int i = 0; i <= steps; ++i) {
    const float k = static_cast<float>(i) / static_cast<float>(steps);
    const float px = std::floor((a.x + dx * k) / grid) * grid, py = std::floor((a.y + dy * k) / grid) * grid;
    FillRect(px - t * 0.5f, py - t * 0.5f, t, t, c);
  }
}

void Graphics::PixelRing(float cx, float cy, float r, float thickness, Color c, float grid) {
  const int n = Clamp(static_cast<int>(2.f * kPi * r / grid), 12, 600);
  const float t = std::max(grid, thickness);
  for (int i = 0; i < n; ++i) {
    const float a = 2.f * kPi * static_cast<float>(i) / static_cast<float>(n);
    const float px = std::floor((cx + std::cos(a) * r) / grid) * grid, py = std::floor((cy + std::sin(a) * r) / grid) * grid;
    FillRect(px - t * 0.5f, py - t * 0.5f, t, t, c);
  }
}

int Graphics::TextWidth(const std::string& s, int scale) const {
  if (s.empty()) return 0;
  return static_cast<int>(s.size()) * (kGlyphW + 1) * scale - scale;
}

void Graphics::TextRaw(const std::string& s, float x, float y, int scale, Color c) {
  float cx = x;
  const float sc = static_cast<float>(scale);
  for (char ch : s) {
    const char* const* g = PixelGlyph(ch);
    if (g) {
      for (int row = 0; row < kGlyphH; ++row) {
        int col = 0;
        while (col < kGlyphW) {
          if (g[row][col] != '#') { ++col; continue; }
          int run = 1;
          while (col + run < kGlyphW && g[row][col + run] == '#') ++run;
          FillRect(cx + static_cast<float>(col) * sc, y + static_cast<float>(row) * sc, static_cast<float>(run) * sc, sc, c);
          col += run;
        }
      }
    }
    cx += static_cast<float>(kGlyphW + 1) * sc;
  }
}

void Graphics::Text(const std::string& s, float x, float y, int scale, Color c, Align align, bool outline) {
  const float w = static_cast<float>(TextWidth(s, scale));
  if (align == Align::Center) x -= w * 0.5f;
  else if (align == Align::Right) x -= w;
  if (outline) {
    const float o = static_cast<float>(std::max(1, scale / 2 + 1));
    const Color dark = Rgb(8, 0, 20, 220);
    TextRaw(s, x - o, y, scale, dark);
    TextRaw(s, x + o, y, scale, dark);
    TextRaw(s, x, y - o, scale, dark);
    TextRaw(s, x, y + o, scale, dark);
  }
  TextRaw(s, x, y, scale, c);
}

}  // namespace gengar

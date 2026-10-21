#pragma once
// SDL2 기반 2D 그리기 래퍼. 모든 도형은 "월드 좌표"로 그리고,
// SetOffset(카메라/흔들림)과 PushTransform(스프라이트 로컬 좌표)이 자동 적용된다.
#include <string>
#include <vector>

#include "core/Types.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace gengar {

enum class Align { Left, Center, Right };

// 도트 스프라이트: 문자 격자 + 팔레트로 만든 텍스처. 확대는 항상 nearest-neighbor(도트가 번지지 않음).
using SpriteId = int;
constexpr SpriteId kNoSprite = -1;
struct PaletteEntry {
  char key;
  Color color;
};

class Graphics {
 public:
  Graphics() = default;
  ~Graphics();
  Graphics(const Graphics&) = delete;
  Graphics& operator=(const Graphics&) = delete;

  bool Init(const char* title, int logicalW, int logicalH);
  void Shutdown();

  void Begin(Color clear);
  void End();

  // 화면 좌표 = 월드 좌표 - offset
  void SetOffset(float x, float y) { ox_ = x; oy_ = y; }
  void SetGlobalAlpha(float a) { galpha_ = a; }

  // 이후 도형은 로컬 좌표: world = origin + (x*scale*flipX, y*scale). 중첩 호출 금지(1단계만 지원).
  void PushTransform(Vec2 origin, float scale = 1.f, int flipX = 1);
  void PopTransform();

  void FillRect(float x, float y, float w, float h, Color c);
  void FillEllipse(float cx, float cy, float rx, float ry, Color c, float rotation = 0.f);
  void FillCircle(float cx, float cy, float r, Color c) { FillEllipse(cx, cy, r, r, c); }
  void FillTriangle(Vec2 a, Vec2 b, Vec2 c, Color col);
  void FillQuad(Vec2 a, Vec2 b, Vec2 c, Vec2 d, Color col);
  void Line(Vec2 a, Vec2 b, float thickness, Color c);
  void DashedLine(Vec2 a, Vec2 b, float thickness, float dash, float gap, Color c);
  void Ring(float cx, float cy, float r, float thickness, Color c);
  void Glow(float cx, float cy, float r, Color c);  // 부드러운 방사형 빛
  void Heart(float x, float y, float scale, bool full);

  // ---- 도트(픽셀) 그리기 ----
  // rows: 같은 길이의 문자열들. '.' = 투명, 그 외 문자는 palette 에서 색을 찾는다.
  SpriteId CreateSprite(const std::vector<std::string>& rows, const std::vector<PaletteEntry>& palette);
  Vec2 SpriteSize(SpriteId id) const;  // 도트 단위 크기
  // (x,y) = 스프라이트 좌상단(월드 좌표). scale = 도트 1개를 화면 몇 px로 그릴지. flash = 흰색 실루엣.
  void DrawSprite(SpriteId id, float x, float y, int scale = 2, bool flipX = false, float alpha = 1.f, bool flash = false);
  // 격자(grid px)에 맞춰 계단식으로 그리는 선/원 (매끈한 선 대신 도트 느낌)
  void PixelLine(Vec2 a, Vec2 b, float thickness, Color c, float grid = 2.f);
  void PixelRing(float cx, float cy, float r, float thickness, Color c, float grid = 2.f);

  // 픽셀 폰트 텍스트. y는 글자 윗변. scale = 픽셀 배율(글자 높이 = 7*scale)
  void Text(const std::string& s, float x, float y, int scale, Color c, Align align = Align::Left,
            bool outline = true);
  int TextWidth(const std::string& s, int scale) const;

 private:
  struct Xf {
    Vec2 origin;
    float scale;
    int flip;
  };

  Vec2 Map(Vec2 p) const;
  float Scale() const { return xf_.empty() ? 1.f : xf_.back().scale; }
  Color Apply(Color c) const { return WithAlpha(c, galpha_); }
  void Tris(const std::vector<Vec2>& screenPts, Color c);
  void TextRaw(const std::string& s, float x, float y, int scale, Color c);

  SDL_Window* window_ = nullptr;
  SDL_Renderer* renderer_ = nullptr;
  float ox_ = 0.f, oy_ = 0.f, galpha_ = 1.f;
  std::vector<Xf> xf_;
  std::vector<Vec2> scratch_;
  struct SpriteTex {
    SDL_Texture* tex = nullptr;
    SDL_Texture* flash = nullptr;
    int w = 0, h = 0;
  };
  std::vector<SpriteTex> sprites_;
};

}  // namespace gengar

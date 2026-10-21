#pragma once
// 내장 5x7 픽셀 폰트 (ASCII 대문자/숫자/기호). 폰트 파일이 필요 없어 웹 빌드가 단순하다.

namespace gengar {

constexpr int kGlyphW = 5;
constexpr int kGlyphH = 7;

// 7개 행 문자열('#'=켜짐, '.'=꺼짐) 배열을 돌려준다. 지원하지 않는 문자는 nullptr.
const char* const* PixelGlyph(char c);

}  // namespace gengar

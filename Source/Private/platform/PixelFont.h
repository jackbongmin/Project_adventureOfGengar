#pragma once
// 내장 5x7 픽셀 폰트 (ASCII 대문자/숫자/기호). 폰트 파일이 필요 없어 웹 빌드가 단순하다.
// 한글은 갈무리7(Galmuri7, SIL OFL 1.1)의 7x7 글리프 중 게임에 쓰는 글자만 HangulGlyphs.cpp 에 구워 둔다.
#include <cstddef>
#include <cstdint>
#include <string>

namespace gengar {

constexpr int kGlyphW = 5;
constexpr int kGlyphH = 7;
constexpr int kHangulAdvance = 8;  // 한글 한 글자가 차지하는 폭 (글리프 7 + 간격 1)

// 7개 행 문자열('#'=켜짐, '.'=꺼짐) 배열을 돌려준다. 지원하지 않는 문자는 nullptr.
const char* const* PixelGlyph(char c);

// 한글 7x7 글리프: 7줄, 각 줄의 최상위 비트(0x80)가 왼쪽 칸. 구워 두지 않은 글자는 nullptr.
const uint8_t* HangulGlyph(uint32_t cp);

// s[i] 부터 UTF-8 한 글자를 읽고 i 를 다음 글자로 옮긴다. 깨진 바이트는 그 바이트 하나로 읽는다.
uint32_t NextCodepoint(const std::string& s, size_t& i);

// 한 글자가 차지하는 폭 (배율 1 기준 픽셀). ASCII 는 6, 그 밖의 글자는 8.
inline int GlyphAdvance(uint32_t cp) { return cp < 0x80 ? kGlyphW + 1 : kHangulAdvance; }

}  // namespace gengar

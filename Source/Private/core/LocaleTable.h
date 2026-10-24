#pragma once
// 영어 → 한국어 문장표 (LocaleKo.cpp). 숫자가 들어가는 문장은 숫자 자리를 '#' 으로 적는다.
// 한국어 쪽에서 '#' 은 영어에 나온 순서대로 숫자를 넣고, "{1}" "{2}" 는 그 번째 숫자를 넣는다.
// 여기에 한글을 추가했으면 tools/fontgen/gen_hangul.py 를 다시 돌려 HangulGlyphs.cpp 에 글자를 구워 넣는다.
#include <cstddef>

namespace gengar {

struct LocaleEntry {
  const char* en;
  const char* ko;
};

extern const LocaleEntry kKorean[];
extern const size_t kKoreanCount;

}  // namespace gengar

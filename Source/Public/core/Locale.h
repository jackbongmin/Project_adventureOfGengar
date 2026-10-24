#pragma once
// 화면 글자 번역. 게임 코드는 영어 문장을 그대로 쓰고, 설정 언어가 한국어면 Graphics::Text 가 여기서 바꿔 그린다.
//  - 문장 전체가 표에 있으면 그대로 바꾼다.
//  - 없으면 숫자를 '#' 으로 바꾼 틀("STAGE #")을 찾아 바꾼 뒤 숫자를 순서대로 다시 넣는다.
//  - 앞뒤 공백은 그대로 둔다 ("NEED " 처럼 폭을 재는 문자열용).
//  - 표에 없으면 영어 그대로.
#include <string>
#include <vector>

namespace gengar {

std::string Localize(const std::string& s);

// 번역한 뒤 공백 기준으로 줄을 나눈다. maxChars = 영어 글자(폭 6) 기준 최대 글자 수. 한글 한 글자는 폭 8.
std::vector<std::string> WrapLocalized(const std::string& text, size_t maxChars);

// UTF-8 글자 수 (타자기 연출처럼 한 글자씩 보여 줄 때)
size_t CharCount(const std::string& s);
// 앞에서 n 글자 (UTF-8 글자 단위)
std::string FirstChars(const std::string& s, size_t n);

}  // namespace gengar

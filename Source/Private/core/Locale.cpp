#include "core/Locale.h"

#include <cctype>
#include <unordered_map>

#include "core/DebugOptions.h"
#include "core/LocaleTable.h"
#include "core/Settings.h"
#include "platform/PixelFont.h"

namespace gengar {
namespace {

const std::unordered_map<std::string, std::string>& KoreanTable() {
  static const std::unordered_map<std::string, std::string> table = [] {
    std::unordered_map<std::string, std::string> m;
    for (size_t i = 0; i < kKoreanCount; ++i) m.emplace(kKorean[i].en, kKorean[i].ko);
    return m;
  }();
  return table;
}

// 숫자(소수점 포함)를 '#' 하나로 바꾼 틀과, 뽑아낸 숫자들
std::string ToPattern(const std::string& s, std::vector<std::string>& numbers) {
  std::string out;
  for (size_t i = 0; i < s.size();) {
    if (!std::isdigit(static_cast<unsigned char>(s[i]))) { out += s[i++]; continue; }
    size_t j = i;
    while (j < s.size() && (std::isdigit(static_cast<unsigned char>(s[j])) ||
                            (s[j] == '.' && j + 1 < s.size() && std::isdigit(static_cast<unsigned char>(s[j + 1])))))
      ++j;
    numbers.push_back(s.substr(i, j - i));
    out += '#';
    i = j;
  }
  return out;
}

// 번역 틀에 숫자를 다시 넣는다. '#' = 다음 숫자, "{n}" = n 번째 숫자 (어순이 바뀔 때)
std::string FillPattern(const std::string& ko, const std::vector<std::string>& numbers) {
  std::string out;
  size_t next = 0;
  for (size_t i = 0; i < ko.size(); ++i) {
    if (ko[i] == '#') { out += next < numbers.size() ? numbers[next++] : "#"; continue; }
    if (ko[i] == '{' && i + 2 < ko.size() && std::isdigit(static_cast<unsigned char>(ko[i + 1])) && ko[i + 2] == '}') {
      const size_t n = static_cast<size_t>(ko[i + 1] - '1');
      out += n < numbers.size() ? numbers[n] : "#";
      i += 2;
      continue;
    }
    out += ko[i];
  }
  return out;
}

std::string Translate(const std::string& s) {
  const size_t a = s.find_first_not_of(' ');
  if (a == std::string::npos) return s;
  const size_t b = s.find_last_not_of(' ');
  const std::string core = s.substr(a, b - a + 1);
  const auto& table = KoreanTable();
  std::string ko;
  if (auto it = table.find(core); it != table.end()) {
    ko = it->second;
  } else {
    std::vector<std::string> numbers;
    const std::string pattern = ToPattern(core, numbers);
    auto pt = numbers.empty() ? table.end() : table.find(pattern);
    if (pt == table.end()) {
      // 영어 단어(대문자 두 글자 이상)가 남아 있으면 번역 누락일 수 있다: log=1 로 실행해서 찾는다
      for (size_t i = 0; i + 1 < core.size(); ++i) {
        if (std::isupper(static_cast<unsigned char>(core[i])) && std::isupper(static_cast<unsigned char>(core[i + 1]))) {
          DebugLog("[locale] no korean: \"%s\"", core.c_str());
          break;
        }
      }
      return s;
    }
    ko = FillPattern(pt->second, numbers);
  }
  return s.substr(0, a) + ko + s.substr(b + 1);
}

}  // namespace

std::string Localize(const std::string& s) {
  if (Prefs().language != Language::Korean || s.empty()) return s;
  // 같은 문자열을 매 프레임 그리므로 결과를 기억해 둔다. 점수처럼 계속 바뀌는 글자가 쌓이면 비운다.
  static std::unordered_map<std::string, std::string> cache;
  if (auto it = cache.find(s); it != cache.end()) return it->second;
  if (cache.size() > 4096) cache.clear();
  return cache.emplace(s, Translate(s)).first->second;
}

std::vector<std::string> WrapLocalized(const std::string& text, size_t maxChars) {
  const std::string s = Localize(text);
  const int maxW = static_cast<int>(maxChars) * (kGlyphW + 1);
  auto width = [](const std::string& t) {
    int w = 0;
    for (size_t i = 0; i < t.size();) w += GlyphAdvance(NextCodepoint(t, i));
    return w;
  };
  std::vector<std::string> lines;
  std::string line;
  size_t i = 0;
  while (i < s.size()) {
    const size_t sp = s.find(' ', i);
    const std::string word = s.substr(i, sp == std::string::npos ? std::string::npos : sp - i);
    if (!line.empty() && width(line) + (kGlyphW + 1) + width(word) > maxW) { lines.push_back(line); line.clear(); }
    line += (line.empty() ? "" : " ") + word;
    if (sp == std::string::npos) break;
    i = sp + 1;
  }
  if (!line.empty()) lines.push_back(line);
  return lines;
}

size_t CharCount(const std::string& s) {
  size_t n = 0;
  for (size_t i = 0; i < s.size(); ++n) NextCodepoint(s, i);
  return n;
}

std::string FirstChars(const std::string& s, size_t n) {
  size_t i = 0;
  for (size_t k = 0; k < n && i < s.size(); ++k) NextCodepoint(s, i);
  return s.substr(0, i);
}

}  // namespace gengar

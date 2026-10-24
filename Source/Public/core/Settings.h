#pragma once
// 플레이어가 메인 메뉴의 SETTINGS 에서 바꾸는 사용자 설정. (Debug() 처럼 전역 접근자로 읽는다.)

namespace gengar {

enum class JumpControl { Fixed, Variable, ShortTap };  // 항상 최대 높이 / 짧게=낮게·꾹=높게 / 짧게 누르면 아주 낮게
enum class Language { English, Korean };  // 화면 글자 언어 (core/Locale 이 번역한다)

struct Settings {
  int volume = 7;                            // 효과음 크기 0~10
  int shake = 2;                             // 화면 흔들림 0 끄기 / 1 약하게 / 2 기본
  JumpControl jump = JumpControl::Variable;  // 점프 방식
  Language language = Language::English;
};

Settings& Prefs();

inline float ShakeScale(const Settings& s) { return s.shake == 0 ? 0.f : s.shake == 1 ? 0.4f : 1.f; }

}  // namespace gengar

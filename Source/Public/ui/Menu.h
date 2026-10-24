#pragma once
// 메인 메뉴와 일시정지 메뉴가 함께 쓰는 메뉴 UI: 항목 한 줄, 도트 테두리 패널, 설정 화면.
#include <initializer_list>
#include <string>

#include "core/Types.h"
#include "platform/Graphics.h"

namespace gengar {

class Game;
class Graphics;

// 항목 한 줄. 선택되면 밝은 색과 좌우로 까딱이는 화살표로 보여 준다.
void DrawMenuRow(Graphics& g, const std::string& text, float x, float y, bool selected, int frame, Color idle = Hex(0xc9b0ee));

// 모서리가 계단처럼 깎인 도트 테두리 패널 (그림자 + 바깥 테두리 + 안쪽 하이라이트)
void DrawPixelPanel(Graphics& g, float x, float y, float w, float h);

// 키보드 키 모양(키캡). label 이 "UP" "DOWN" "LEFT" "RIGHT" 면 화살표를 그린다. 그린 폭을 돌려준다.
float DrawKeyCap(Graphics& g, const std::string& label, float x, float y);
// 키 안내 한 줄: [키][키] 설명   [키] 설명 ... 을 centerX 기준 가운데 정렬로 그린다.
struct KeyHint {
  std::initializer_list<const char*> keys;
  const char* label;
};
void DrawKeyHints(Graphics& g, std::initializer_list<KeyHint> hints, float centerX, float y);
// 코인 아이콘 + 금액. (x, y) = 왼쪽 위(Align::Left) 또는 오른쪽 위(Align::Right). 그린 폭을 돌려준다.
float DrawCoinAmount(Graphics& g, int amount, float x, float y, int scale, Color color, Align align = Align::Left);

// 화면 오른쪽 아래에 게임 버전. Game::Frame 이 모든 화면 위에 그린다.
void DrawVersion(Graphics& g);

// ---- 설정 화면 (효과음 크기 / 화면 흔들림 / 점프 방식 / 언어 / BACK) ----
enum SettingItem { kSettingVolume, kSettingShake, kSettingJump, kSettingLanguage, kSettingBack, kSettingCount };

// 커서 이동과 값 변경을 처리한다. ESC 또는 BACK 을 고르면 true.
bool UpdateSettingsMenu(Game& game, int& cursor);
// 항목 줄들을 (x, y) 부터 폭 w 로 그리고(약 236px 높이), 고른 항목의 설명을 hintY 에 쓴다.
void DrawSettingsMenu(Graphics& g, int cursor, int frame, float x, float y, float w, float hintY);

}  // namespace gengar

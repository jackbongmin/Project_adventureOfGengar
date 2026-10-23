#pragma once
// 메인 메뉴와 일시정지 메뉴가 함께 쓰는 메뉴 UI: 항목 한 줄, 도트 테두리 패널, 설정 화면.
#include <string>

#include "core/Types.h"

namespace gengar {

class Game;
class Graphics;

// 항목 한 줄. 선택되면 밝은 색과 좌우로 까딱이는 화살표로 보여 준다.
void DrawMenuRow(Graphics& g, const std::string& text, float x, float y, bool selected, int frame, Color idle = Hex(0xc9b0ee));

// 모서리가 계단처럼 깎인 도트 테두리 패널 (그림자 + 바깥 테두리 + 안쪽 하이라이트)
void DrawPixelPanel(Graphics& g, float x, float y, float w, float h);

// ---- 설정 화면 (효과음 크기 / 화면 흔들림 / 점프 방식 / BACK) ----
enum SettingItem { kSettingVolume, kSettingShake, kSettingJump, kSettingBack, kSettingCount };

// 커서 이동과 값 변경을 처리한다. ESC 또는 BACK 을 고르면 true.
bool UpdateSettingsMenu(Game& game, int& cursor);
// 항목들을 y0 부터 그린다. 점프 항목이 선택되면 hintY 에 설명을 쓴다.
void DrawSettingsMenu(Graphics& g, int cursor, int frame, float y0, float hintY);

}  // namespace gengar

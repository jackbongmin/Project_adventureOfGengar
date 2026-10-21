#pragma once
// 레벨 배경/타일/깃발 그리기 (내부 전용 헤더)
#include "world/Level.h"

namespace gengar {

class Graphics;

// 호출 전 gfx.SetOffset(흔들림) 상태, 배경은 화면 좌표로 그려진다.
void RenderLevelBackground(Graphics& gfx, const Level& level, float camX, int frame);
// 호출 전 gfx.SetOffset(카메라 + 흔들림) 상태, 월드 좌표로 그린다.
void RenderLevelTiles(Graphics& gfx, const Level& level, float camX, int frame);

}  // namespace gengar

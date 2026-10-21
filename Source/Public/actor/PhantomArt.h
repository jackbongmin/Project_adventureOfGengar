#pragma once
// 주인공(보라색 유령)과 섀도우볼 그리기. 플레이어/타이틀/상점/엔딩 화면에서 공유한다.
#include "core/Types.h"

namespace gengar {

class Graphics;

void DrawPhantom(Graphics& gfx, Vec2 center, int facing, float time, float scale = 1.f, float alpha = 1.f);
void DrawShadowBall(Graphics& gfx, Vec2 center, float radius, float time);

}  // namespace gengar

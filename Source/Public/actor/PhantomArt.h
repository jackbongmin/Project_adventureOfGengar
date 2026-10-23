#pragma once
// 주인공(보라색 유령)과 섀도우볼 그리기. 플레이어/타이틀/상점/엔딩/시네마틱 화면에서 공유한다.
#include "core/Types.h"

namespace gengar {

class Graphics;

// 시네마틱/엔딩용 꾸미기. 기본값이면 평소 모습 그대로다.
struct PhantomStyle {
  bool happy = false;    // 눈웃음 + 볼터치 표정
  bool hat = false;      // 보물 모자를 쓰고 있음
  float hatGlow = 0.f;   // 0~1: 모자 주위로 빛과 반짝이
  Vec2 squash{1.f, 1.f};  // 스쿼시/스트레치 (가로, 세로 배율). 발끝을 기준으로 늘어난다.
};

void DrawPhantom(Graphics& gfx, Vec2 center, int facing, float time, float scale = 1.f, float alpha = 1.f,
                 const PhantomStyle& style = {});
// 머리 위 모자가 놓이는 자리(모자 챙의 아래 중앙). 둥실거림까지 반영한다.
Vec2 PhantomHatAnchor(Vec2 center, int facing, float time, float scale = 1.f, Vec2 squash = {1.f, 1.f});
// bottomCenter = 모자 챙 아래 중앙. glow > 0 이면 빛과 반짝이를 함께 그린다.
void DrawTreasureHat(Graphics& gfx, Vec2 bottomCenter, float scale, float glow, float time, float alpha = 1.f);
// radius 에 맞는 정수 도트 배율로 그리고, 둘레에 radius 에 비례하는 빛을 깐다. alpha < 1 이면 잔상용으로 빛 없이 반투명하게.
void DrawShadowBall(Graphics& gfx, Vec2 center, float radius, float time, float alpha = 1.f);

}  // namespace gengar

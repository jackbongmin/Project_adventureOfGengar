#pragma once
// 게임에서 쓰는 모든 도트 스프라이트의 ID 모음. 시작할 때 Init() 한 번으로 텍스처를 만든다.
// 격자 데이터는 Private/art/SpriteData.cpp, 색(팔레트)은 Private/art/Art.cpp.
#include "platform/Graphics.h"

namespace gengar::art {

constexpr int kPixel = 2;  // 도트 1개 = 화면 2px (게임 기본 배율)

struct TileSet {
  SpriteId groundTop = kNoSprite, dirt = kNoSprite, brick = kNoSprite;
  SpriteId crumble = kNoSprite, crumbleHot = kNoSprite, spike = kNoSprite;
};

struct Assets {
  SpriteId phantom[2];
  SpriteId slime[2];
  SpriteId bat[2];
  SpriteId houndCalm[2];
  SpriteId houndAlert[2];
  SpriteId caster[2];
  SpriteId coin[4];
  SpriteId heartFull = kNoSprite, heartEmpty = kNoSprite;
  SpriteId ball[2];
  SpriteId orb = kNoSprite;
  SpriteId wave[2];
  SpriteId pennant[3];
  SpriteId moon = kNoSprite;
  TileSet tiles[3];     // 스테이지(테마)별
  SpriteId boss[3];     // 보스 단계별 색
  SpriteId bossHand[3];
};

void Init(Graphics& gfx);
const Assets& A();

}  // namespace gengar::art

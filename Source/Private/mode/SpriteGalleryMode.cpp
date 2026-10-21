#include "mode/SpriteGalleryMode.h"

#include <string>

#include "art/Art.h"
#include "core/Config.h"
#include "core/Game.h"
#include "platform/Graphics.h"
#include "platform/Input.h"

namespace gengar {

void SpriteGalleryMode::Update(Game& game) {
  ++frame_;
  if (game.In().Pressed(Action::Pause) || game.In().Pressed(Action::Confirm)) game.GoTitle();
}

void SpriteGalleryMode::Render(Graphics& g) {
  const art::Assets& a = art::A();
  g.FillRect(0, 0, static_cast<float>(cfg::kScreenW), static_cast<float>(cfg::kScreenH), Hex(0x2a2440));
  g.Text("SPRITE GALLERY  (ESC TO BACK)", 16, 8, 2, Hex(0xffffff));

  // 한 줄에 스프라이트들을 간격을 두고 늘어놓는다. 반환값 = 다음 x 좌표.
  float x = 16.f;
  auto put = [&](SpriteId id, float y, int scale) {
    const Vec2 sz = g.SpriteSize(id);
    g.DrawSprite(id, x, y, scale);
    x += sz.x * static_cast<float>(scale) + 12.f;
  };

  // 캐릭터/적
  const float y1 = 34.f;
  for (int i = 0; i < 2; ++i) put(a.phantom[i], y1, 3);
  for (int i = 0; i < 2; ++i) put(a.slime[i], y1 + 30.f, 3);
  for (int i = 0; i < 2; ++i) put(a.bat[i], y1 + 20.f, 3);
  for (int i = 0; i < 2; ++i) put(a.houndCalm[i], y1 + 20.f, 3);
  put(a.houndAlert[0], y1 + 20.f, 3);
  for (int i = 0; i < 2; ++i) put(a.caster[i], y1 + 8.f, 3);

  // 아이템/투사체
  x = 16.f;
  const float y2 = 130.f;
  for (int i = 0; i < 4; ++i) put(a.coin[i], y2, 4);
  put(a.heartFull, y2, 4);
  put(a.heartEmpty, y2, 4);
  for (int i = 0; i < 2; ++i) put(a.ball[i], y2, 4);
  put(a.orb, y2, 4);
  for (int i = 0; i < 2; ++i) put(a.wave[i], y2, 4);
  for (int i = 0; i < 3; ++i) put(a.pennant[i], y2, 3);
  put(a.moon, y2 - 6.f, 2);

  // 타일 (테마 3종)
  for (int t = 0; t < 3; ++t) {
    x = 16.f;
    const float y = 190.f + static_cast<float>(t) * 52.f;
    const art::TileSet& ts = a.tiles[t];
    for (SpriteId id : {ts.groundTop, ts.dirt, ts.brick, ts.crumble, ts.crumbleHot, ts.spike}) put(id, y, 3);
  }

  // 시네마틱용: 행복한 표정 + 보물 모자 (타일 오른쪽)
  x = 420.f;
  for (int i = 0; i < 2; ++i) put(a.phantomHappy[i], 190.f, 3);
  put(a.treasureHat, 200.f, 3);

  // 보스 (단계별 색) + 손
  x = 16.f;
  const float y4 = 350.f;
  for (int i = 0; i < 3; ++i) put(a.boss[i], y4, 2);
  for (int i = 0; i < 3; ++i) put(a.bossHand[i], y4 + 40.f, 3);
  g.Text("BOSS PHASE 1-3", 16, 520, 1, Hex(0xc9b0ee), Align::Left, false);
}

}  // namespace gengar

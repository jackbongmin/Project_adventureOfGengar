#include "art/Art.h"

#include "art/SpriteData.h"
#include "data/CardId.h"

namespace gengar::art {
namespace {

Assets g_assets;
using Pal = std::vector<PaletteEntry>;

// ---- 팔레트 (문자 -> 색). 격자의 문자와 1:1로 대응한다. ----
const Pal kPhantom = {{'o', Hex(0x241046)}, {'d', Hex(0x4a2480)}, {'p', Hex(0x7a45c4)}, {'l', Hex(0xa47af0)},
                      {'w', Hex(0xffffff)}, {'r', Hex(0xff2a4a)}, {'k', Hex(0x1a0830)}};
const Pal kPhantomHappy = {{'o', Hex(0x241046)}, {'d', Hex(0x4a2480)}, {'p', Hex(0x7a45c4)}, {'l', Hex(0xa47af0)},
                           {'w', Hex(0xffffff)}, {'k', Hex(0x1a0830)}, {'b', Hex(0xff7ab8)}};
const Pal kTreasureHat = {{'o', Hex(0x2a1405)}, {'b', Hex(0x3a1a6a)}, {'B', Hex(0x5a2a9a)}, {'y', Hex(0xffd24a)},
                          {'h', Hex(0xfff3b0)}, {'d', Hex(0xd9992a)}, {'r', Hex(0xff2a4a)}, {'w', Hex(0xffffff)}};
const Pal kSlime = {{'o', Hex(0x0e4a2c)}, {'g', Hex(0x4fd98a)}, {'h', Hex(0x8ff5b8)}, {'d', Hex(0x2fa868)},
                    {'w', Hex(0xffffff)}, {'W', Hex(0xe9fff2)}, {'k', Hex(0x0e1a2a)}};
const Pal kBat = {{'o', Hex(0x2a0a18)}, {'b', Hex(0x3a0f24)}, {'w', Hex(0xa3224f)}, {'W', Hex(0xd4476f)},
                  {'v', Hex(0x6e1236)}, {'y', Hex(0xffe14a)}, {'f', Hex(0xffffff)}};
const Pal kCaster = {{'o', Hex(0x1a0a2e)}, {'p', Hex(0x5a2a8a)}, {'l', Hex(0x7a4ab0)}, {'d', Hex(0x3a1a60)},
                     {'k', Hex(0x0a0412)}, {'r', Hex(0xff2a4a)}, {'b', Hex(0x6b4a2a)}, {'m', Hex(0xff5ca8)}, {'w', Hex(0xffffff)}};
const Pal kCoin = {{'o', Hex(0x7a4a08)}, {'y', Hex(0xffd24a)}, {'h', Hex(0xfff3b0)}, {'d', Hex(0xd9992a)}};
const Pal kHeartFull = {{'o', Hex(0x5a1030)}, {'R', Hex(0xff4f86)}, {'W', Hex(0xffd0e0)}};
const Pal kHeartEmpty = {{'o', Hex(0x3a2a55)}, {'E', Hex(0x231338)}};
const Pal kBall = {{'r', Hex(0xb36bff)}, {'m', Hex(0x5a2aa8)}, {'d', Hex(0x14062e)}, {'h', Hex(0xd9b8ff)}, {'l', Hex(0x8a4ad8)}};
const Pal kOrb = {{'o', Hex(0x8a1a5a)}, {'p', Hex(0xff5ca8)}, {'w', Hex(0xffffff)}};
const Pal kWave = {{'p', Hex(0xc58bff)}, {'w', Hex(0xffffff)}, {'o', Hex(0x5a2a98)}};
const Pal kPennant = {{'l', Hex(0xc38bff)}, {'p', Hex(0xa24bff)}, {'o', Hex(0x4a1a8a)}};
// 상점 아이콘 공용 팔레트
const Pal kUiIcon = {{'o', Hex(0x1a0830)}, {'w', Hex(0xffffff)}, {'s', Hex(0xb8b8dc)}, {'y', Hex(0xffd24a)},
                     {'d', Hex(0xb07020)}, {'R', Hex(0xff4f86)}, {'W', Hex(0xffd0e0)}, {'g', Hex(0x6fe08a)},
                     {'c', Hex(0x7ae0ff)}, {'k', Hex(0xff7ab8)}, {'K', Hex(0xffe0f0)}, {'p', Hex(0x9a5cff)},
                     {'m', Hex(0x4a2490)}, {'r', Hex(0xff2a4a)}, {'h', Hex(0xfff3b0)}};
const Pal kMoon = {{'m', Hex(0xf3e7ff)}, {'M', Hex(0xffffff)}, {'d', Hex(0xc9b0ee)}, {'c', Hex(0xb6a0dc)}};

Pal HoundPal(Color eye) {
  return {{'o', Hex(0x1a0c2e)}, {'p', Hex(0x4d3282)}, {'l', Hex(0x6f4ca8)}, {'d', Hex(0x2e1a52)},
          {'v', Hex(0x9a66e0)}, {'e', eye}, {'w', Hex(0xffffff)}};
}

struct TileTheme {
  uint32_t dirt, dirt2, dirtLight, top, top2, brick, brickHi, brickMortar, brickShade;
};
const TileTheme kTileThemes[3] = {
    {0x3b2a4e, 0x2d1f3e, 0x4d3a64, 0x7be08a, 0x4aa860, 0x7a4b8f, 0x9a6bb0, 0x4a2a5a, 0x5a3570},
    {0x262b4a, 0x1b1f38, 0x343a60, 0x6fb8d9, 0x3d7fa3, 0x4a5a8f, 0x6a7ab0, 0x2a3560, 0x38467a},
    {0x2d1230, 0x210b25, 0x43204a, 0xc25ad1, 0x8c2f9c, 0x6d2a73, 0x8e4a95, 0x3d1442, 0x55205a},
};

Pal GroundPal(const TileTheme& t) {
  return {{'D', Hex(t.dirt)}, {'S', Hex(t.dirt2)}, {'L', Hex(t.dirtLight)}, {'T', Hex(t.top)}, {'t', Hex(t.top2)}};
}
Pal BrickPal(const TileTheme& t) {
  return {{'B', Hex(t.brick)}, {'H', Hex(t.brickHi)}, {'M', Hex(t.brickMortar)}, {'S', Hex(t.brickShade)}};
}
const Pal kCrumble = {{'B', Hex(0xa8794e)}, {'H', Hex(0xc9985f)}, {'S', Hex(0x7a5230)}, {'K', Hex(0x3a1e0e)}};
const Pal kCrumbleHot = {{'B', Hex(0xd98a5a)}, {'H', Hex(0xffb27a)}, {'S', Hex(0xa8613a)}, {'K', Hex(0x5a2a14)}};
const Pal kSpike = {{'L', Hex(0xdfe3f5)}, {'S', Hex(0x8a90b8)}, {'o', Hex(0x3a3f60)}};

struct BossColors {
  uint32_t accent, ring, eye, iris;
};
const BossColors kBossPhases[3] = {
    {0xb46cff, 0x7a3fd0, 0x9a2ac0, 0xe060ff},  // 1단계: 보라
    {0xff6cd8, 0xd03fb0, 0xb01a90, 0xff60c8},  // 2단계: 분홍
    {0xff6c6c, 0xd03f3f, 0xb01a1a, 0xff7040},  // 3단계: 붉은색
};
Pal BossPal(const BossColors& c) {
  return {{'c', Hex(0x1a0a30)}, {'m', Hex(0x2d1254)}, {'a', Hex(c.accent)}, {'x', Hex(c.ring)}, {'v', Hex(0x04010a)},
          {'e', Hex(c.eye)}, {'f', Hex(c.iris)}, {'w', Hex(0xeadfff)}, {'k', Hex(0x04010a)}};
}

}  // namespace

void Init(Graphics& g) {
  Assets& a = g_assets;
  a.phantom[0] = g.CreateSprite(data::Phantom0(), kPhantom);
  a.phantom[1] = g.CreateSprite(data::Phantom1(), kPhantom);
  a.phantomHappy[0] = g.CreateSprite(data::PhantomHappy0(), kPhantomHappy);
  a.phantomHappy[1] = g.CreateSprite(data::PhantomHappy1(), kPhantomHappy);
  a.treasureHat = g.CreateSprite(data::TreasureHat(), kTreasureHat);
  a.slime[0] = g.CreateSprite(data::Slime0(), kSlime);
  a.slime[1] = g.CreateSprite(data::Slime1(), kSlime);
  a.bat[0] = g.CreateSprite(data::Bat0(), kBat);
  a.bat[1] = g.CreateSprite(data::Bat1(), kBat);
  const Pal calm = HoundPal(Hex(0xff9a4a)), alert = HoundPal(Hex(0xff2a4a));
  a.houndCalm[0] = g.CreateSprite(data::Hound0(), calm);
  a.houndCalm[1] = g.CreateSprite(data::Hound1(), calm);
  a.houndAlert[0] = g.CreateSprite(data::Hound0(), alert);
  a.houndAlert[1] = g.CreateSprite(data::Hound1(), alert);
  a.caster[0] = g.CreateSprite(data::Caster0(), kCaster);
  a.caster[1] = g.CreateSprite(data::Caster1(), kCaster);
  a.coin[0] = g.CreateSprite(data::Coin0(), kCoin);
  a.coin[1] = g.CreateSprite(data::Coin1(), kCoin);
  a.coin[2] = g.CreateSprite(data::Coin2(), kCoin);
  a.coin[3] = g.CreateSprite(data::Coin3(), kCoin);
  a.heartFull = g.CreateSprite(data::HeartFull(), kHeartFull);
  a.heartEmpty = g.CreateSprite(data::HeartEmpty(), kHeartEmpty);
  a.ball[0] = g.CreateSprite(data::Ball0(), kBall);
  a.ball[1] = g.CreateSprite(data::Ball1(), kBall);
  a.orb = g.CreateSprite(data::Orb0(), kOrb);
  a.wave[0] = g.CreateSprite(data::Wave0(), kWave);
  a.wave[1] = g.CreateSprite(data::Wave1(), kWave);
  a.pennant[0] = g.CreateSprite(data::Pennant0(), kPennant);
  a.pennant[1] = g.CreateSprite(data::Pennant1(), kPennant);
  a.pennant[2] = g.CreateSprite(data::Pennant2(), kPennant);
  a.moon = g.CreateSprite(data::Moon(), kMoon);
  for (int i = 0; i < kCardCount; ++i) a.cardIcon[i] = g.CreateSprite(data::CardIconRows(static_cast<CardId>(i)), kUiIcon);
  a.departIcon = g.CreateSprite(data::IconDepart(), kUiIcon);

  for (int i = 0; i < 3; ++i) {
    TileSet& t = a.tiles[i];
    const Pal ground = GroundPal(kTileThemes[i]), brick = BrickPal(kTileThemes[i]);
    t.groundTop = g.CreateSprite(data::TileGroundTop(), ground);
    t.dirt = g.CreateSprite(data::TileDirt(), ground);
    t.brick = g.CreateSprite(data::TileBrick(), brick);
    t.crumble = g.CreateSprite(data::TileCrumble(), kCrumble);
    t.crumbleHot = g.CreateSprite(data::TileCrumble(), kCrumbleHot);
    t.spike = g.CreateSprite(data::TileSpike(), kSpike);
    const Pal bossPal = BossPal(kBossPhases[i]);
    a.boss[i] = g.CreateSprite(data::Boss(), bossPal);
    a.bossHand[i] = g.CreateSprite(data::BossHand(), bossPal);
  }
}

const Assets& A() { return g_assets; }

}  // namespace gengar::art

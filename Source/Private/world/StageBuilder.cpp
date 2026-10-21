// 스테이지 3개의 지형/적/아이템 배치 데이터.
// 새 스테이지를 만들려면 BuildStage에 분기를 추가하면 된다. (열=타일 x, 행=타일 y, 바닥 윗면=15행)
#include <initializer_list>

#include "world/Level.h"

namespace gengar {
namespace {

constexpr float T = static_cast<float>(cfg::kTile);
constexpr int kGround = cfg::kGroundRow;

// 스테이지 구성 보조 함수 모음
class Builder {
 public:
  explicit Builder(Level& level) : L(level) {}

  void Pit(int a, int b) { for (int x = a; x <= b; ++x) { L.Set(x, kGround, Tile::Empty); L.Set(x, kGround + 1, Tile::Empty); } }
  void Plat(int a, int b, int row) { for (int x = a; x <= b; ++x) L.Set(x, row, Tile::Brick); }
  void Spike(int a, int b) { for (int x = a; x <= b; ++x) L.Set(x, kGround - 1, Tile::Spike); }
  void Stack(int col, int h) { for (int k = 1; k <= h; ++k) L.Set(col, kGround - k, Tile::Brick); }
  void Crumble(int a, int b, int row) { L.AddCrumble(a, b, row); }
  void Ceiling() { for (int x = 0; x < L.WidthTiles(); ++x) { L.Set(x, 0, Tile::Ground); L.Set(x, 1, Tile::Ground); } }
  void Walls() { for (int y = 0; y < cfg::kRows; ++y) { L.Set(0, y, Tile::Ground); L.Set(L.WidthTiles() - 1, y, Tile::Ground); } }

  void Slime(int c) { L.Spawns().push_back({SpawnType::Slime, {c * T + 2.f, kGround * T - 22.f}}); }
  void Hound(int c) { L.Spawns().push_back({SpawnType::Hound, {c * T, kGround * T - 24.f}}); }
  void Caster(int c) { L.Spawns().push_back({SpawnType::Caster, {c * T + 3.f, kGround * T - 32.f}}); }
  void Bat(int c, int row, float range) { L.Spawns().push_back({SpawnType::Bat, {c * T, row * T}, range}); }
  void Coin(int c, int row, int count = 1) {
    for (int i = 0; i < count; ++i) L.Spawns().push_back({SpawnType::Coin, {(c + i) * T + 16.f, row * T + 16.f}});
  }
  void CoinArc(int c, int row) {
    const float dy[5] = {0.f, 0.8f, 1.2f, 0.8f, 0.f};
    for (int i = 0; i < 5; ++i) L.Spawns().push_back({SpawnType::Coin, {(c + i) * T + 16.f, (row - dy[i]) * T + 16.f}});
  }
  void Heart(int c, int row) { L.Spawns().push_back({SpawnType::Heart, {c * T + 16.f, row * T + 16.f}}); }
  void Flag(int c) { L.SetFlag({c * T + 8.f, 10 * T, 16.f, 5 * T}); }

 private:
  Level& L;
};

template <class F>
void Each(std::initializer_list<int> list, F f) { for (int v : list) f(v); }

void BuildGrassland(Level& level) {
  Builder b(level);
  b.Pit(26, 28); b.Pit(50, 53); b.Pit(74, 77); b.Pit(96, 99);
  b.Plat(12, 15, 12); b.Plat(34, 37, 12); b.Plat(40, 43, 9); b.Plat(60, 63, 12); b.Plat(66, 69, 9); b.Plat(84, 87, 12);
  Each({18, 31, 45, 57, 70, 82, 90, 102, 108}, [&](int c) { b.Slime(c); });
  Each({38, 80}, [&](int c) { b.Caster(c); });
  Each({48, 62, 94}, [&](int c) { b.Hound(c); });
  b.CoinArc(25, 12); b.CoinArc(49, 12); b.CoinArc(73, 12); b.CoinArc(95, 12);
  b.Coin(12, 11, 4); b.Coin(34, 11, 4); b.Coin(40, 8, 4); b.Coin(60, 11, 4); b.Coin(66, 8, 4); b.Coin(84, 11, 4);
  b.Coin(5, 14, 3); b.Coin(30, 14, 4); b.Coin(46, 14, 3); b.Coin(80, 14, 3); b.Coin(108, 14, 4);
  b.Heart(41, 8);
  b.Stack(104, 1); b.Stack(105, 2); b.Stack(106, 3); b.Stack(107, 4);
  b.Flag(112);
}

void BuildCave(Level& level) {
  Builder b(level);
  b.Ceiling();
  b.Pit(20, 23); b.Pit(40, 43); b.Pit(62, 66); b.Pit(85, 88); b.Pit(105, 109); b.Pit(124, 127);
  // 절벽 사이 발판은 밟으면 무너진다
  b.Crumble(21, 22, 13); b.Crumble(41, 42, 13); b.Crumble(63, 65, 13); b.Crumble(86, 87, 13); b.Crumble(106, 108, 13); b.Crumble(125, 126, 13);
  b.Plat(13, 16, 12); b.Plat(46, 49, 12); b.Plat(48, 51, 9); b.Plat(68, 71, 12); b.Plat(90, 93, 12); b.Plat(92, 95, 9); b.Plat(112, 115, 12); b.Plat(130, 133, 12);
  b.Spike(30, 32); b.Spike(52, 54); b.Spike(74, 76); b.Spike(97, 99); b.Spike(117, 119);
  Each({12, 27, 35, 47, 58, 70, 80, 91, 101, 112, 121, 135}, [&](int c) { b.Slime(c); });
  Each({37, 56, 78, 102, 121}, [&](int c) { b.Caster(c); });
  Each({25, 45, 60, 92, 113, 131}, [&](int c) { b.Hound(c); });
  b.Bat(15, 9, 100); b.Bat(37, 8, 100); b.Bat(56, 8, 110); b.Bat(79, 8, 100); b.Bat(98, 8, 100); b.Bat(118, 8, 100); b.Bat(138, 9, 100);
  b.CoinArc(20, 11); b.CoinArc(40, 12); b.CoinArc(62, 9); b.CoinArc(85, 12); b.CoinArc(105, 9); b.CoinArc(124, 12);
  b.Coin(13, 11, 4); b.Coin(46, 11, 4); b.Coin(48, 8, 4); b.Coin(68, 11, 4); b.Coin(92, 8, 4); b.Coin(112, 11, 4); b.Coin(130, 11, 4);
  b.Coin(6, 14, 3); b.Coin(28, 14, 2); b.Coin(60, 14, 2); b.Coin(100, 14, 3);
  b.Heart(50, 8); b.Heart(131, 11);
  b.Stack(139, 1); b.Stack(140, 2); b.Stack(141, 3); b.Stack(142, 4); b.Stack(143, 5);
  b.Flag(147);
}

void BuildBossArena(Level& level) {
  Builder b(level);
  b.Walls();
  b.Plat(4, 7, 12); b.Plat(22, 25, 12); b.Plat(12, 17, 9);
  b.Heart(5, 11); b.Heart(23, 11);
  b.Coin(13, 8, 4);
  level.Spawns().push_back({SpawnType::Boss, {cfg::kScreenW * 0.5f, -120.f}});
}

}  // namespace

Level BuildStage(int stageIndex) {
  switch (stageIndex) {
    case 0: { Level l(120, 0, "STAGE 1 - MISTY FIELDS"); BuildGrassland(l); return l; }
    case 1: { Level l(150, 1, "STAGE 2 - WHISPERING CAVE"); BuildCave(l); return l; }
    default: { Level l(30, 2, "FINAL - THRONE OF THE VOID"); BuildBossArena(l); return l; }
  }
}

}  // namespace gengar

// 스테이지 3개의 지형/적/아이템 배치 데이터. 스테이지 1·2 는 매복 결투장, 점프 버섯, 정예 적, 보물상자로 구성한다.
// 새 스테이지를 만들려면 BuildStage에 분기를 추가하면 된다. (열=타일 x, 행=타일 y, 바닥 윗면=15행)
#include <initializer_list>
#include <vector>

#include "world/Level.h"

namespace gengar {
namespace {

constexpr float T = static_cast<float>(cfg::kTile);
constexpr int kGround = cfg::kGroundRow;

// 스테이지 구성 보조 함수 모음.
// 적은 지금 고른 목록(평소에는 레벨, BeginArena~EndArena 사이에서는 그 결투장의 웨이브)에 들어간다.
class Builder {
 public:
  explicit Builder(Level& level) : L(level), out_(&level.Spawns()) {}

  void Pit(int a, int b) { for (int x = a; x <= b; ++x) { L.Set(x, kGround, Tile::Empty); L.Set(x, kGround + 1, Tile::Empty); } }
  void Plat(int a, int b, int row) { for (int x = a; x <= b; ++x) L.Set(x, row, Tile::OneWay); }  // 뜬 발판: 아래에서 통과해 올라간다
  void Spike(int a, int b) { for (int x = a; x <= b; ++x) L.Set(x, kGround - 1, Tile::Spike); }
  void Stack(int col, int h) { for (int k = 1; k <= h; ++k) L.Set(col, kGround - k, Tile::Brick); }
  void Crumble(int a, int b, int row) { L.AddCrumble(a, b, row); }
  void Spring(int c) { L.Set(c, kGround, Tile::Spring); }  // 바닥에 박힌 점프 버섯: 밟고 지나가기만 해도 약 9칸 높이로 튀어 오른다
  void Ceiling() { for (int x = 0; x < L.WidthTiles(); ++x) { L.Set(x, 0, Tile::Ground); L.Set(x, 1, Tile::Ground); } }
  void Walls() { for (int y = 0; y < cfg::kRows; ++y) { L.Set(0, y, Tile::Ground); L.Set(L.WidthTiles() - 1, y, Tile::Ground); } }

  void Slime(int c) { out_->push_back({SpawnType::Slime, {c * T + 2.f, kGround * T - 22.f}}); }
  void Hound(int c) { out_->push_back({SpawnType::Hound, {c * T, kGround * T - 24.f}}); }
  void Caster(int c) { out_->push_back({SpawnType::Caster, {c * T + 3.f, kGround * T - 32.f}}); }
  void CasterOn(int c, int row) { out_->push_back({SpawnType::Caster, {c * T + 3.f, row * T - 32.f}}); }  // 뜬 발판 위
  // 결투장 박쥐는 7~8행: 가운데 발판(9행)에 올라서면 맞힐 수 있는 높이
  void Hopper(int c) { out_->push_back({SpawnType::Hopper, {c * T + 2.f, kGround * T - 22.f}}); }
  void Bat(int c, int row, float range) { out_->push_back({SpawnType::Bat, {c * T, row * T}, range}); }
  void Elite() { out_->back().elite = true; }  // 바로 앞에 놓은 적을 정예로

  // 매복 결투장: BeginArena(c0, c1) 뒤에 Wave() 마다 적을 놓고 EndArena(). c0, c1 열은 싸우는 동안 벽이 된다.
  void BeginArena(int c0, int c1) { L.Arenas().push_back({c0, c1, {}}); }
  void Wave() { L.Arenas().back().waves.emplace_back(); out_ = &L.Arenas().back().waves.back(); }
  void EndArena() { out_ = &L.Spawns(); }

  void Coin(int c, int row, int count = 1) {
    for (int i = 0; i < count; ++i) L.Spawns().push_back({SpawnType::Coin, {(c + i) * T + 16.f, row * T + 16.f}});
  }
  void CoinArc(int c, int row) {
    const float dy[5] = {0.f, 0.8f, 1.2f, 0.8f, 0.f};
    for (int i = 0; i < 5; ++i) L.Spawns().push_back({SpawnType::Coin, {(c + i) * T + 16.f, (row - dy[i]) * T + 16.f}});
  }
  void Heart(int c, int row) { L.Spawns().push_back({SpawnType::Heart, {c * T + 16.f, row * T + 16.f}}); }
  void Chest(int c, int row) { L.Spawns().push_back({SpawnType::Chest, {c * T + 16.f, row * T + 16.f}}); }
  void Flag(int c) { L.SetFlag({c * T + 8.f, 10 * T, 16.f, 5 * T}); }

 private:
  Level& L;
  std::vector<SpawnPoint>* out_;
};

template <class F>
void Each(std::initializer_list<int> list, F f) { for (int v : list) f(v); }

// 스테이지 1: 몸풀기 -> 결투장 1 (2웨이브) -> 버섯으로 넘는 절벽과 높은 길의 숨은 상자, 정예 늑대 -> 결투장 2 (3웨이브) -> 깃발
void BuildGrassland(Level& level) {
  Builder b(level);
  // ---- 몸풀기: 첫 버섯은 바로 위 발판의 코인으로 안내한다 ----
  b.Plat(12, 15, 12);
  b.Slime(10); b.Slime(17);
  b.Spring(6); b.Plat(5, 9, 7); b.CoinArc(5, 6);
  b.Coin(12, 11, 4);
  b.Pit(21, 23); b.CoinArc(20, 12);

  // ---- 결투장 1 (26~55) ----
  b.Plat(31, 34, 12); b.Plat(47, 50, 12); b.Plat(38, 43, 9);
  b.BeginArena(26, 55);
  b.Wave(); b.Slime(33); b.Slime(41); b.Slime(49);
  b.Wave(); b.Hound(30); b.Hound(52); b.Slime(40); b.Hopper(45);
  b.EndArena();
  b.Coin(39, 8, 4);

  // ---- 가운데 길 (56~99) ----
  b.Spring(57); b.Pit(59, 62); b.CoinArc(58, 8);          // 버섯으로 절벽 넘기
  b.Plat(65, 68, 12); b.CasterOn(66, 12);
  b.Slime(70); b.Bat(72, 8, 120); b.Hopper(76);
  b.Hound(81); b.Elite();                                  // 정예 늑대
  b.Spring(86); b.Plat(84, 93, 7); b.Coin(85, 6, 4); b.Chest(91, 6);  // 높은 길의 숨은 상자
  b.Heart(88, 13);
  b.Pit(95, 97); b.CoinArc(94, 12);

  // ---- 결투장 2 (100~129) ----
  b.Plat(104, 107, 12); b.Plat(122, 125, 12); b.Plat(111, 118, 9);
  b.BeginArena(100, 129);
  b.Wave(); b.Hound(103); b.Hound(126); b.Bat(108, 8, 100); b.Bat(120, 8, 100);
  b.Wave(); b.Caster(110); b.Caster(119); b.Slime(105); b.Hopper(124);
  b.Wave(); b.Hound(115); b.Elite(); b.Slime(103); b.Slime(126);
  b.EndArena();

  // ---- 마무리 (130~159) ----
  b.Slime(135); b.Hopper(141); b.Bat(138, 9, 100);
  b.Plat(137, 140, 12); b.Coin(137, 11, 4);
  b.Stack(148, 1); b.Stack(149, 2); b.Stack(150, 3); b.Stack(151, 4);
  b.Flag(155);
}

// 스테이지 2: 무너지는 다리와 가시 사이를 버섯으로 넘고, 결투장 2곳(3웨이브씩)과 정예 마법사, 천장 아래 숨은 상자
void BuildCave(Level& level) {
  Builder b(level);
  b.Ceiling();
  // ---- 입구 ----
  b.Plat(13, 16, 12); b.Slime(10); b.Caster(18); b.Bat(15, 9, 100);
  b.Coin(13, 11, 4);
  b.Pit(22, 25); b.Crumble(23, 24, 13); b.CoinArc(22, 11);
  b.Spring(28); b.Spike(30, 32); b.CoinArc(29, 8);       // 가시밭을 버섯으로 넘는다
  b.Hound(35); b.Bat(36, 8, 100);

  // ---- 결투장 1 (40~69) ----
  b.Plat(44, 47, 12); b.Plat(62, 65, 12); b.Plat(52, 57, 9);
  b.BeginArena(40, 69);
  b.Wave(); b.Slime(45); b.Hopper(55); b.Slime(64); b.Bat(50, 8, 90);
  b.Wave(); b.Caster(48); b.Caster(60); b.Hound(43); b.Hound(66);
  b.Wave(); b.Caster(54); b.Elite(); b.Bat(46, 8, 90); b.Bat(62, 8, 90);
  b.EndArena();
  b.Coin(53, 8, 4);

  // ---- 가운데 길 (70~113) ----
  b.Pit(72, 75); b.Crumble(73, 74, 13); b.CoinArc(72, 11);
  b.Spring(78); b.Spike(80, 82); b.Plat(84, 87, 12); b.CasterOn(85, 12);
  b.Slime(89); b.Elite();                                  // 정예 슬라임 (쓰러지면 셋으로 갈라진다)
  b.Hopper(92); b.Elite();                                 // 정예 두꺼비 (착지 충격파)
  b.Spring(95); b.Plat(93, 101, 7); b.Coin(94, 6, 4); b.Chest(99, 6);  // 천장 아래 숨은 상자
  b.Heart(97, 13);
  b.Pit(104, 107); b.Crumble(105, 106, 13); b.Bat(106, 9, 100);
  b.Hound(110);

  // ---- 결투장 2 (114~143) ----
  b.Plat(118, 121, 12); b.Plat(136, 139, 12); b.Plat(124, 133, 9);
  b.BeginArena(114, 143);
  b.Wave(); b.Hound(117); b.Hound(140); b.Hopper(128);
  b.Wave(); b.Caster(120); b.Caster(137); b.Bat(122, 7, 80); b.Bat(134, 7, 80); b.Slime(128);
  b.Wave(); b.Hound(128); b.Elite(); b.Caster(118); b.Elite(); b.Bat(126, 7, 80);
  b.EndArena();

  // ---- 출구 (144~179) ----
  b.Spring(148); b.Spike(150, 152); b.CoinArc(149, 8);
  b.Hopper(156); b.Caster(161); b.Bat(158, 9, 100);
  b.Plat(158, 161, 12); b.Coin(158, 11, 4); b.Heart(160, 11);
  b.Stack(168, 1); b.Stack(169, 2); b.Stack(170, 3); b.Stack(171, 4); b.Stack(172, 5);
  b.Flag(176);
}

void BuildBossArena(Level& level) {
  Builder b(level);
  b.Walls();
  // 좌우 대칭 3층 '왕좌' 발판: 아래층(양 끝 + 가운데) -> 가운데층 둘 -> 맨 위 하나.
  // 한 층 높이 3칸은 점프(약 4칸)로 오를 수 있고, 가로 빈칸은 1~2칸이다.
  // 충격파는 아래층, 가로 탄막은 층을 옮겨 다니며 피한다. 플레이어 공은 뜬 발판을 뚫는다.
  b.Plat(2, 5, 12); b.Plat(13, 16, 12); b.Plat(24, 27, 12);
  b.Plat(8, 11, 9); b.Plat(18, 21, 9);
  b.Plat(13, 16, 6);
  b.Heart(3, 11); b.Heart(25, 11);
  b.Coin(13, 5, 4);
  level.Spawns().push_back({SpawnType::Boss, {cfg::kScreenW * 0.5f, -120.f}});
}

}  // namespace

Level BuildStage(int stageIndex) {
  switch (stageIndex) {
    case 0: { Level l(160, 0, "STAGE 1 - MISTY FIELDS"); BuildGrassland(l); return l; }
    case 1: { Level l(180, 1, "STAGE 2 - WHISPERING CAVE"); BuildCave(l); return l; }
    default: { Level l(30, 2, "FINAL - THRONE OF THE VOID"); BuildBossArena(l); return l; }
  }
}

}  // namespace gengar

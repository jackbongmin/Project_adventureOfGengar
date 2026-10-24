#pragma once
// 타일맵 + 스폰 지점 + 무너지는 발판. 스테이지 데이터는 StageBuilder.cpp에서 만든다.
#include <string>
#include <unordered_map>
#include <vector>

#include "core/Config.h"
#include "core/Types.h"

namespace gengar {

class World;

// OneWay: 공중에 뜬 발판. 아래에서 뛰어오르면 통과하고 위에서만 밟힌다.
// Barrier: 매복 결투장이 진행되는 동안만 생기는 마법 벽 (World 가 놓고 치운다).
// Spring: 점프 버섯. 플레이어가 위에 내려앉으면 높이 튀어 오른다.
enum class Tile : uint8_t { Empty = 0, Ground = 1, Brick = 2, Spike = 3, Crumble = 4, OneWay = 5, Barrier = 6, Spring = 7 };

enum class SpawnType { Slime, Bat, Hound, Caster, Hopper, Boss, Coin, Heart, Chest };

struct SpawnPoint {
  SpawnType type;
  Vec2 pos;          // Slime/Hound/Caster/Bat/Hopper = 좌상단, Coin/Heart/Boss/Chest = 중심
  float range = 0.f;  // Bat 순찰 범위
  bool elite = false;  // 정예: 체력 3배, 빛나는 테두리, 경험치·코인 많이
};

// 매복 결투장: 플레이어가 c0~c1 열 안쪽으로 들어오면 양 끝 열에 마법 벽이 생기고 적이 웨이브로 나온다.
// 웨이브를 모두 잡으면 벽이 사라지고 가운데에 보물상자가 떨어진다. 폭은 화면 한 장(30칸)에 맞춘다.
struct ArenaDef {
  int c0 = 0, c1 = 0;
  std::vector<std::vector<SpawnPoint>> waves;
};

// 밟으면 잠시 뒤 떨어지고, 시간이 지나면 복구되는 발판 묶음(가로로 이어진 타일들)
class CrumbleGroup {
 public:
  enum class State { Idle, Shake, Gone };
  CrumbleGroup(int c0, int c1, int row) : c0(c0), c1(c1), row(row) {}
  int c0, c1, row;
  State state = State::Idle;
  int timer = 0;
};

class Level {
 public:
  Level(int widthTiles, int theme, std::string name);

  int Theme() const { return theme_; }
  const std::string& Name() const { return name_; }
  int WidthTiles() const { return width_; }
  float PixelWidth() const { return static_cast<float>(width_ * cfg::kTile); }

  Tile At(int c, int r) const;
  bool Solid(int c, int r) const;  // 지면/벽돌/무너지는 발판/뜬 발판. 좌우 바깥은 벽, 위아래 바깥은 빈 공간.
  bool OneWay(int c, int r) const { return At(c, r) == Tile::OneWay; }  // 위에서만 설 수 있는 발판 (캐릭터 충돌에서 따로 다룬다)
  void Set(int c, int r, Tile t);

  std::vector<SpawnPoint>& Spawns() { return spawns_; }
  const std::vector<SpawnPoint>& Spawns() const { return spawns_; }
  std::vector<ArenaDef>& Arenas() { return arenas_; }
  const std::vector<ArenaDef>& Arenas() const { return arenas_; }

  void SetFlag(const Rect& r) { flag_ = r; hasFlag_ = true; }
  bool HasFlag() const { return hasFlag_; }
  const Rect& Flag() const { return flag_; }

  void AddCrumble(int c0, int c1, int row);
  const CrumbleGroup* CrumbleAt(int c, int r) const;
  void UpdateCrumbles(World& world);

 private:
  int width_;
  int theme_;
  std::string name_;
  std::vector<Tile> tiles_;
  std::vector<SpawnPoint> spawns_;
  std::vector<ArenaDef> arenas_;
  Rect flag_;
  bool hasFlag_ = false;
  std::vector<CrumbleGroup> crumbles_;
  std::unordered_map<int, size_t> crumbIndex_;
};

// 스테이지 번호(0~2)에 해당하는 레벨 생성. (StageBuilder.cpp)
Level BuildStage(int stageIndex);

}  // namespace gengar

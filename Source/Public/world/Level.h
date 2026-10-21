#pragma once
// 타일맵 + 스폰 지점 + 무너지는 발판. 스테이지 데이터는 StageBuilder.cpp에서 만든다.
#include <string>
#include <unordered_map>
#include <vector>

#include "core/Config.h"
#include "core/Types.h"

namespace gengar {

class World;

enum class Tile : uint8_t { Empty = 0, Ground = 1, Brick = 2, Spike = 3, Crumble = 4 };

enum class SpawnType { Slime, Bat, Hound, Caster, Boss, Coin, Heart };

struct SpawnPoint {
  SpawnType type;
  Vec2 pos;          // Slime/Hound/Caster/Bat = 좌상단, Coin/Heart/Boss = 중심
  float range = 0.f;  // Bat 순찰 범위
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
  bool Solid(int c, int r) const;  // 지면/벽돌/무너지는 발판. 좌우 바깥은 벽, 위아래 바깥은 빈 공간.
  void Set(int c, int r, Tile t);

  std::vector<SpawnPoint>& Spawns() { return spawns_; }
  const std::vector<SpawnPoint>& Spawns() const { return spawns_; }

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
  Rect flag_;
  bool hasFlag_ = false;
  std::vector<CrumbleGroup> crumbles_;
  std::unordered_map<int, size_t> crumbIndex_;
};

// 스테이지 번호(0~2)에 해당하는 레벨 생성. (StageBuilder.cpp)
Level BuildStage(int stageIndex);

}  // namespace gengar

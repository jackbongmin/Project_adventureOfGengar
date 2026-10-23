#pragma once
// 파티클, 떠오르는 텍스트, 충격파 링, 추락 조각, 화면 흔들림/플래시를 관리한다.
#include <string>
#include <vector>

#include "core/Settings.h"
#include "core/Types.h"

namespace gengar {

class Graphics;

class Effects {
 public:
  void Burst(Vec2 p, int count, Color color, float speed = 3.f, float life = 30.f);
  void Trail(Vec2 p, Color color, float size, float life);
  void Dust(Vec2 feet, int count, float pushX = 0.f);  // 발밑 먼지 (점프/착지/달리기/급정거)
  void Pop(Vec2 p, const std::string& text, Color color = Color{});
  void AddRing(Vec2 center);
  void AddChunk(Vec2 topLeft);  // 무너지는 발판 조각
  void Shake(float amount) { shake_ = std::max(shake_, amount * ShakeScale(Prefs())); }
  void Flash() { flash_ = 14; }

  float ShakeAmount() const { return shake_; }
  float FlashAmount() const { return static_cast<float>(flash_) / 14.f; }

  void Update();
  void Render(Graphics& gfx) const;
  void Clear();

 private:
  struct Particle { Vec2 p, v; float life, maxLife, size; Color color; float gravity; };
  struct FloatText { Vec2 p; std::string text; Color color; int life; };
  struct Ring { Vec2 c; float r; int life; };
  struct Chunk { Vec2 p; float vy, rot, vr; };

  std::vector<Particle> particles_;
  std::vector<FloatText> texts_;
  std::vector<Ring> rings_;
  std::vector<Chunk> chunks_;
  float shake_ = 0.f;
  int flash_ = 0;
};

}  // namespace gengar

#include "actor/hazard/PillarHazard.h"

#include "actor/Player.h"
#include "core/Config.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "world/World.h"

namespace gengar {

namespace {
constexpr float kGroundY = static_cast<float>(cfg::kGroundRow * cfg::kTile);
}

PillarHazard::PillarHazard(float centerX, int warnFrames) : Actor(ActorKind::Hazard), warn_(warnFrames) {
  pos = {centerX, kGroundY};
}

Rect PillarHazard::Bounds() const { return {pos.x - 16.f, kGroundY - kHeight, 32.f, kHeight}; }

void PillarHazard::Update(World& w) {
  if (++t_ == warn_) {
    w.Fx().Shake(6.f);
    w.Snd().Play(Sfx::PillarHit);
  }
  if (t_ > warn_ + kActiveFrames) { Kill(); return; }
  if (t_ >= warn_) {
    Player& p = w.GetPlayer();
    if (p.IsAlive() && Bounds().Overlaps(p.Bounds())) {
      p.TakeDamage(w, DamageInfo{1.f, Sign(p.Bounds().Center().x - pos.x) != 0 ? Sign(p.Bounds().Center().x - pos.x) : 1});
    }
  }
}

void PillarHazard::Render(Graphics& g, const World&) {
  const float x = pos.x;
  if (t_ < warn_) {  // 예고: 바닥 마법진 + 옅은 기둥 윤곽
    const float k = static_cast<float>(t_) / static_cast<float>(warn_);
    g.PixelRing(x, kGroundY - 2.f, 14.f + 10.f * k, 4.f, Rgb(255, 80, 200, static_cast<int>((0.3f + 0.5f * k) * 255)));
    g.FillRect(x - 16, kGroundY - kHeight, 32, kHeight, Rgb(255, 80, 200, static_cast<int>((0.06f + 0.12f * k) * 255)));
  } else {  // 분출: 아래로 갈수록 진한 기둥
    const float k = static_cast<float>(t_ - warn_) / static_cast<float>(kActiveFrames);
    const float w = 17.f * (1.f - k * 0.4f);
    for (int i = 0; i < 4; ++i) {
      const float y0 = kGroundY - kHeight + kHeight * 0.25f * static_cast<float>(i);
      const int a = 70 + i * 50;
      g.FillRect(x - w, y0, w * 2.f, kHeight * 0.25f, Rgb(190 + i * 20, 90 - i * 5, 255 - i * 10, a));
    }
    g.FillRect(x - 3, kGroundY - kHeight, 6, kHeight, Hex(0xffffff));
  }
}

}  // namespace gengar

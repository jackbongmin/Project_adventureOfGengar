#include "mode/VictoryMode.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

#include "actor/PhantomArt.h"
#include "art/Art.h"
#include "core/Config.h"
#include "core/Game.h"
#include "core/Session.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "platform/Input.h"

namespace gengar {
namespace {

constexpr float kW = static_cast<float>(cfg::kScreenW);
constexpr float kH = static_cast<float>(cfg::kScreenH);

// ---- 타임라인 (프레임) ----
constexpr int kHatLand = 150;    // 모자가 머리 위에 내려앉는다
constexpr int kCloseUp = 220;    // 행복한 얼굴 클로즈업 (~260 확대)
constexpr int kTheEnd = 460;     // THE END
constexpr int kCanLeave = 490;   // 이후 Enter로 타이틀

const CutsceneBeat kBeats[] = {
    {0, kCloseUp, "", "THE TREASURE HAT IS FINALLY BACK!"},
    {kCloseUp, kTheEnd, "PHANTOM", "HEHE... WELCOME BACK, MY HAT!\nI'LL NEVER LET YOU GO AGAIN!"},
};
constexpr int kBeatCount = static_cast<int>(sizeof(kBeats) / sizeof(kBeats[0]));

float Ramp(int t, int a, int b) { return std::clamp(static_cast<float>(t - a) / static_cast<float>(b - a), 0.f, 1.f); }
float EaseOut(float k) { return 1.f - (1.f - k) * (1.f - k); }
float EaseInOut(float k) { return k * k * (3.f - 2.f * k); }

void DrawDawn(Graphics& g, int t) {
  DrawNightSky(g, t, Hex(0x1a0838), Lerp(Hex(0x4a1a78), Hex(0xff9ab8), 0.35f));
}

// 아래에서 떠오르는 하트들
void DrawFloatingHearts(Graphics& g, int t, float alpha) {
  const SpriteId heart = art::A().heartFull;
  for (int i = 0; i < 9; ++i) {
    const float life = std::fmod(static_cast<float>(t) * 1.4f + static_cast<float>(i) * 61.f, 560.f);
    const float x = 80.f + static_cast<float>((i * 211) % 800) + std::sin(static_cast<float>(t) * 0.04f + static_cast<float>(i)) * 18.f;
    const float y = kH - life;
    g.DrawSprite(heart, std::floor(x), std::floor(y), 3, false, alpha * std::min(1.f, life / 120.f));
  }
}

}  // namespace

void VictoryMode::Update(Game& game) {
  prev_ = t_;
  ++t_;
  score_ = game.Sess().score;
  Input& in = game.In();
  if (t_ < kTheEnd && skip_.Update(in)) t_ = kTheEnd;
  if (in.Pressed(Action::Confirm)) {
    const int beat = BeatAt(kBeats, kBeatCount, t_);
    if (beat >= 0) t_ = std::max(t_, kBeats[beat].end);
    else if (t_ >= kCanLeave) { game.GoTitle(); return; }
  }

  Audio& snd = game.Snd();
  if (Crossed(kHatLand)) snd.Play(Sfx::StageClear);
  if (Crossed(kCloseUp + 40)) snd.Play(Sfx::Heart);
  if (Crossed(kTheEnd)) snd.Play(Sfx::Coin);
  const int beat = BeatAt(kBeats, kBeatCount, t_);
  if (beat >= 0 && t_ % 4 == 0) {
    const int typed = (t_ - kBeats[beat].start) / 2;
    if (typed < static_cast<int>(std::strlen(kBeats[beat].text))) snd.Tone(900.f, 900.f, 0.025f, Wave::Square, 0.02f);
  }
}

void VictoryMode::Render(Graphics& g) {
  const int t = t_;
  const float ft = static_cast<float>(t);
  DrawDawn(g, t);

  if (t < kTheEnd) {
    // 1) 빛나는 모자가 하늘에서 내려와 머리 위에 앉는다  2) 행복한 얼굴로 확대
    const float zoom = EaseInOut(Ramp(t, kCloseUp, kCloseUp + 40));
    const float scale = 3.f + zoom * 2.f;
    const Vec2 center{kW * 0.5f, 300.f + zoom * 10.f};
    const bool landed = t >= kHatLand;
    if (zoom > 0.f) DrawFloatingHearts(g, t, zoom);
    g.Glow(center.x, center.y - 40.f, 160.f + zoom * 80.f, Rgb(255, 226, 122, landed ? 90 : 40));

    Vec2 pc = center;
    if (landed && t < kHatLand + 20) pc.y -= std::sin(Ramp(t, kHatLand, kHatLand + 20) * kPi) * 18.f;  // 기뻐서 폴짝
    PhantomStyle style;
    style.happy = landed;
    style.hat = landed;
    style.hatGlow = landed ? 1.f - 0.5f * zoom : 0.f;
    DrawPhantom(g, pc, 1, ft, scale, 1.f, style);
    if (!landed) {
      const Vec2 anchor = PhantomHatAnchor(pc, 1, ft, scale);
      const float k = EaseOut(Ramp(t, 20, kHatLand));
      DrawTreasureHat(g, {anchor.x + std::sin(ft * 0.07f) * 30.f * (1.f - k), -40.f + (anchor.y + 40.f) * k}, scale, 1.f, ft);
    }
    if (t >= kHatLand && t < kHatLand + 10) g.FillRect(0, 0, kW, kH, Rgb(255, 240, 200, 150));

    DrawLetterbox(g, 1.f);
    const int beat = BeatAt(kBeats, kBeatCount, t);
    if (beat >= 0) DrawDialog(g, kBeats[beat], t);
    skip_.Render(g);
    const float fade = std::max(1.f - Ramp(t, 0, 30), Ramp(t, kTheEnd - 20, kTheEnd));
    if (fade > 0.f) g.FillRect(0, 0, kW, kH, Rgb(0, 0, 0, static_cast<int>(fade * 255.f)));
    return;
  }

  // 3) THE END
  DrawFloatingHearts(g, t, 0.5f);
  PhantomStyle style;
  style.happy = true;
  style.hat = true;
  style.hatGlow = 0.6f;
  DrawPhantom(g, {kW * 0.5f, 170.f}, 1, ft, 2.6f, 1.f, style);
  g.Text("THE END", kW * 0.5f, 250, 9, Hex(0xffe27a), Align::Center);
  g.Text("VOIDMAW IS DEFEATED, AND PHANTOM'S HAT IS HOME AGAIN.", kW * 0.5f, 330, 2, Hex(0xffffff), Align::Center);
  g.Text("FINAL SCORE  " + std::to_string(score_), kW * 0.5f, 370, 4, Hex(0xd9b8ff), Align::Center);
  if (t >= kCanLeave && (t / 30) % 2 == 0) g.Text("PRESS ENTER - TITLE", kW * 0.5f, 450, 3, Hex(0xffe27a), Align::Center);
  const float fade = 1.f - Ramp(t, kTheEnd, kTheEnd + 30);
  if (fade > 0.f) g.FillRect(0, 0, kW, kH, Rgb(0, 0, 0, static_cast<int>(fade * 255.f)));
}

}  // namespace gengar

#include "mode/Cutscene.h"

#include <algorithm>
#include <cmath>

#include "core/Config.h"
#include "core/Locale.h"
#include "platform/Graphics.h"
#include "platform/Input.h"

namespace gengar {
namespace {

constexpr float kW = static_cast<float>(cfg::kScreenW);
constexpr float kH = static_cast<float>(cfg::kScreenH);

}  // namespace

bool SkipHold::Update(const Input& in) {
  if (fired_) return false;
  held_ = in.Down(Action::Skip) ? held_ + 1 : std::max(0, held_ - 3);
  if (held_ < kHoldFrames) return false;
  fired_ = true;
  return true;
}

void SkipHold::Render(Graphics& g) const {
  const float k = static_cast<float>(held_) / static_cast<float>(kHoldFrames);
  const Color text = held_ > 0 ? Hex(0xffe27a) : Rgb(201, 176, 238, 170);
  g.Text("HOLD C TO SKIP", kW - 24.f, 14.f, 2, text, Align::Right);
  constexpr float bw = 168.f;
  g.FillRect(kW - 24.f - bw, 34.f, bw, 6.f, Rgb(20, 8, 40, 200));
  if (k > 0.f) g.FillRect(kW - 24.f - bw, 34.f, std::floor(bw * k / 4.f) * 4.f, 6.f, Hex(0xffe27a));
}

int BeatAt(const CutsceneBeat* beats, int count, int t) {
  for (int i = 0; i < count; ++i) {
    if (t >= beats[i].start && t < beats[i].end) return i;
  }
  return -1;
}

void DrawDialog(Graphics& g, const CutsceneBeat& beat, int t, int blinkT) {
  if (beat.text == nullptr || beat.text[0] == '\0') return;
  const std::string full = Localize(beat.text);  // 한 글자씩 보여 주므로 번역을 먼저 한다
  const int total = static_cast<int>(CharCount(full));
  const int shown = std::clamp((t - beat.start) / kFramesPerChar, 0, total);
  constexpr float x = 60.f, y = kH - 112.f, w = kW - 120.f, h = 80.f;
  g.FillRect(x, y, w, h, Rgb(14, 4, 32, 225));
  g.FillRect(x, y, w, 4, Hex(0xa47af0));
  g.FillRect(x, y + h - 4, w, 4, Hex(0x4a2480));
  const bool hasSpeaker = beat.speaker != nullptr && beat.speaker[0] != '\0';
  if (hasSpeaker) {
    const int sw = g.TextWidth(beat.speaker, 2) + 20;
    g.FillRect(x + 16.f, y - 18.f, static_cast<float>(sw), 24.f, Hex(0x4a2480));
    g.Text(beat.speaker, x + 26.f, y - 12.f, 2, Hex(0xffe27a), Align::Left, false);
  }
  // 줄바꿈 문자로 최대 두 줄
  const std::string visible = FirstChars(full, static_cast<size_t>(shown));
  const size_t nl = visible.find('\n');
  const bool twoLines = full.find('\n') != std::string::npos;
  const float ty = twoLines ? y + 16.f : y + 30.f;
  g.Text(visible.substr(0, nl), x + 24.f, ty, 3, Hex(0xffffff));
  if (nl != std::string::npos) g.Text(visible.substr(nl + 1), x + 24.f, ty + 30.f, 3, Hex(0xffffff));
  if (shown == total &&((blinkT < 0 ? t : blinkT) / 20) % 2 == 0) g.FillRect(x + w - 30.f, y + h - 22.f, 10, 6, Hex(0xffe27a));
}

void DrawLetterbox(Graphics& g, float amount) {
  const float bar = std::floor(36.f * std::clamp(amount, 0.f, 1.f));
  if (bar <= 0.f) return;
  g.FillRect(0, 0, kW, bar, Rgb(0, 0, 0));
  g.FillRect(0, kH - bar, kW, bar, Rgb(0, 0, 0));
}

void DrawNightSky(Graphics& g, int frame, Color top, Color bottom) {
  for (float y = 0; y < kH; y += 8.f) g.FillRect(0, y, kW, 8.f, Lerp(top, bottom, y / kH));
  for (int i = 0; i < 70; ++i) {
    const float a = 0.3f + 0.5f * std::sin(static_cast<float>(frame) * 0.04f + static_cast<float>(i));
    g.FillRect(static_cast<float>((i * 151) % cfg::kScreenW), static_cast<float>((i * 67) % 380), 2, 2,
               Rgb(255, 255, 255, static_cast<int>(std::max(0.f, a) * 255)));
  }
}

}  // namespace gengar

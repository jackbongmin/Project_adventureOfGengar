#include "ui/Menu.h"

#include <cmath>

#include "core/Config.h"
#include "core/Game.h"
#include "core/Settings.h"
#include "core/Version.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "platform/Input.h"

namespace gengar {
namespace {

int Wrap(int v, int n) { return (v % n + n) % n; }

const char* JumpName(JumpControl j) {
  return j == JumpControl::Fixed ? "FIXED" : j == JumpControl::Variable ? "VARIABLE" : "SHORT TAP";
}
const char* JumpHint(JumpControl j) {
  return j == JumpControl::Fixed ? "ALWAYS JUMP FULL HEIGHT"
         : j == JumpControl::Variable ? "TAP = LOW JUMP, HOLD = HIGH JUMP"
                                      : "TAP = VERY LOW JUMP, HOLD = HIGH JUMP";
}
const char* ShakeName(int s) { return s == 0 ? "OFF" : s == 1 ? "LOW" : "FULL"; }

// 네 모서리를 도트 하나(p)만큼 깎은 사각형
void NotchedRect(Graphics& g, float x, float y, float w, float h, float p, Color c) {
  g.FillRect(x + p, y, w - 2.f * p, h, c);
  g.FillRect(x, y + p, w, h - 2.f * p, c);
}

}  // namespace

void DrawMenuRow(Graphics& g, const std::string& text, float x, float y, bool selected, int frame, Color idle) {
  g.Text(text, x, y, 3, selected ? Hex(0xffe27a) : idle, Align::Center);
  if (selected) {
    const float bob = std::floor(std::sin(static_cast<float>(frame) * 0.2f) * 3.f);
    const float half = static_cast<float>(text.size()) * 9.f + 26.f;  // scale 3 글자 폭(약 18px)의 절반
    g.Text(">", x - half + bob, y, 3, Hex(0xffe27a), Align::Center);
    g.Text("<", x + half - bob, y, 3, Hex(0xffe27a), Align::Center);
  }
}

void DrawPixelPanel(Graphics& g, float x, float y, float w, float h) {
  constexpr float p = 4.f;  // 도트 한 칸
  NotchedRect(g, x + 2.f * p, y + 2.f * p, w, h, p, Rgb(0, 0, 0, 130));  // 그림자
  NotchedRect(g, x, y, w, h, p, Hex(0x0b0316));                             // 바깥 테두리
  NotchedRect(g, x + p, y + p, w - 2.f * p, h - 2.f * p, p, Hex(0xb48aff)); // 밝은 테두리
  NotchedRect(g, x + 2.f * p, y + 2.f * p, w - 4.f * p, h - 4.f * p, p, Hex(0x5a2a9a));
  NotchedRect(g, x + 3.f * p, y + 3.f * p, w - 6.f * p, h - 6.f * p, p, Hex(0x1c0a34));  // 본문
  // 왼쪽 위에 빛 반사 도트, 오른쪽 아래에 어두운 도트
  g.FillRect(x + 4.f * p, y + 4.f * p, 3.f * p, p, Hex(0x3e1d6a));
  g.FillRect(x + 4.f * p, y + 5.f * p, p, p, Hex(0x3e1d6a));
  g.FillRect(x + w - 7.f * p, y + h - 5.f * p, 3.f * p, p, Hex(0x12061f));
  g.FillRect(x + w - 5.f * p, y + h - 6.f * p, p, p, Hex(0x12061f));
}

void DrawVersion(Graphics& g) {
  g.Text(version::kLabel, static_cast<float>(cfg::kScreenW) - 12.f, static_cast<float>(cfg::kScreenH) - 22.f, 2, Hex(0x8a74b0), Align::Right);
}

bool UpdateSettingsMenu(Game& game, int& cursor) {
  Input& in = game.In();
  Settings& s = Prefs();
  if (in.Pressed(Action::Pause)) { game.Snd().Play(Sfx::Select); return true; }
  if (in.Pressed(Action::Up)) { cursor = Wrap(cursor - 1, kSettingCount); game.Snd().Play(Sfx::Select); }
  if (in.Pressed(Action::Down)) { cursor = Wrap(cursor + 1, kSettingCount); game.Snd().Play(Sfx::Select); }
  int dir = 0;  // 좌우로 값 변경, Enter 는 다음 값으로
  if (in.Pressed(Action::Left)) dir = -1;
  if (in.Pressed(Action::Right) || in.Pressed(Action::Confirm)) dir = 1;
  if (dir == 0) return false;
  switch (cursor) {
    case kSettingVolume:
      s.volume = in.Pressed(Action::Confirm) ? (s.volume + 1) % 11 : Clamp(s.volume + dir, 0, 10);
      game.Snd().SetVolume(static_cast<float>(s.volume) / 10.f);
      game.Snd().Play(Sfx::Coin);  // 바뀐 크기를 바로 들려 준다
      break;
    case kSettingShake: s.shake = Wrap(s.shake + dir, 3); game.Snd().Play(Sfx::Select); break;
    case kSettingJump: s.jump = static_cast<JumpControl>(Wrap(static_cast<int>(s.jump) + dir, 3)); game.Snd().Play(Sfx::Jump); break;
    case kSettingBack:
      if (in.Pressed(Action::Confirm)) { game.Snd().Play(Sfx::Select); return true; }
      break;
  }
  return false;
}

void DrawSettingsMenu(Graphics& g, int cursor, int frame, float y0, float hintY) {
  constexpr float cx = static_cast<float>(cfg::kScreenW) * 0.5f;
  const Settings& s = Prefs();
  std::string bar;
  for (int i = 0; i < 10; ++i) bar += i < s.volume ? "I" : ".";
  DrawMenuRow(g, "VOLUME  " + bar, cx, y0, cursor == kSettingVolume, frame);
  DrawMenuRow(g, std::string("SCREEN SHAKE  ") + ShakeName(s.shake), cx, y0 + 50.f, cursor == kSettingShake, frame);
  DrawMenuRow(g, std::string("JUMP  ") + JumpName(s.jump), cx, y0 + 100.f, cursor == kSettingJump, frame);
  DrawMenuRow(g, "BACK", cx, y0 + 170.f, cursor == kSettingBack, frame);
  if (cursor == kSettingJump) g.Text(JumpHint(s.jump), cx, hintY, 2, Hex(0xe6d6ff), Align::Center);
}

}  // namespace gengar

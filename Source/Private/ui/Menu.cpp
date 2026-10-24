#include "ui/Menu.h"

#include <algorithm>
#include <cmath>

#include "art/Art.h"
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
// 언어 이름은 번역하지 않고 각 언어로 적는다 ("ENGLISH" 는 번역표에 없어서 그대로 나온다)
const char* LanguageName(int l) { return l == static_cast<int>(Language::Korean) ? "한국어" : "ENGLISH"; }

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
    const float half = static_cast<float>(g.TextWidth(text, 3)) * 0.5f + 26.f;
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

float DrawKeyCap(Graphics& g, const std::string& label, float x, float y) {
  constexpr float p = 2.f, h = 26.f;  // 도트 한 칸, 키 높이 (윗면 + 아랫단)
  const bool arrow = label == "UP" || label == "DOWN" || label == "LEFT" || label == "RIGHT";
  const float w = arrow ? 26.f : std::max(26.f, static_cast<float>(g.TextWidth(label, 2)) + 14.f);
  NotchedRect(g, x, y, w, h, p, Hex(0x12061f));                       // 테두리
  NotchedRect(g, x + p, y + p, w - 2.f * p, h - 2.f * p, p, Hex(0x7a5ab0));  // 아랫단 (입체감)
  NotchedRect(g, x + p, y + p, w - 2.f * p, h - 4.f * p - 2.f, p, Hex(0xeee2ff));  // 윗면
  const Color ink = Hex(0x2a1048);
  const float cx = x + w * 0.5f, cy = y + 10.f;
  if (label == "UP") g.FillTriangle({cx, cy - 5.f}, {cx - 6.f, cy + 4.f}, {cx + 6.f, cy + 4.f}, ink);
  else if (label == "DOWN") g.FillTriangle({cx - 6.f, cy - 4.f}, {cx + 6.f, cy - 4.f}, {cx, cy + 5.f}, ink);
  else if (label == "LEFT") g.FillTriangle({cx - 5.f, cy}, {cx + 4.f, cy - 6.f}, {cx + 4.f, cy + 6.f}, ink);
  else if (label == "RIGHT") g.FillTriangle({cx + 5.f, cy}, {cx - 4.f, cy - 6.f}, {cx - 4.f, cy + 6.f}, ink);
  else g.Text(label, cx, y + 4.f, 2, ink, Align::Center, false);
  return w;
}

void DrawKeyHints(Graphics& g, std::initializer_list<KeyHint> hints, float centerX, float y) {
  constexpr float kKeyGap = 4.f, kLabelGap = 8.f, kHintGap = 28.f;
  auto capWidth = [&](const char* k) {
    const std::string s = k;
    return (s == "UP" || s == "DOWN" || s == "LEFT" || s == "RIGHT") ? 26.f : std::max(26.f, static_cast<float>(g.TextWidth(s, 2)) + 14.f);
  };
  float total = 0.f;
  for (const KeyHint& h : hints) {
    for (const char* k : h.keys) total += capWidth(k) + kKeyGap;
    total += kLabelGap - kKeyGap + static_cast<float>(g.TextWidth(h.label, 2)) + kHintGap;
  }
  total -= kHintGap;
  float x = std::floor(centerX - total * 0.5f);
  for (const KeyHint& h : hints) {
    for (const char* k : h.keys) x += DrawKeyCap(g, k, x, y) + kKeyGap;
    x += kLabelGap - kKeyGap;
    g.Text(h.label, x, y + 6.f, 2, Hex(0xc9b0ee));
    x += static_cast<float>(g.TextWidth(h.label, 2)) + kHintGap;
  }
}

float DrawCoinAmount(Graphics& g, int amount, float x, float y, int scale, Color color, Align align) {
  const std::string text = std::to_string(amount);
  const float icon = 8.f * static_cast<float>(scale), gap = 2.f * static_cast<float>(scale);
  const float w = icon + gap + static_cast<float>(g.TextWidth(text, scale));
  const float left = align == Align::Right ? x - w : x;
  // 글자 높이(7도트)와 코인(8도트)의 가운데를 맞춘다
  g.DrawSprite(art::A().coin[0], left, y - static_cast<float>(scale) * 0.5f, scale);
  g.Text(text, left + icon + gap, y, scale, color);
  return w;
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
    case kSettingLanguage: s.language = static_cast<Language>(Wrap(static_cast<int>(s.language) + dir, 2)); game.Snd().Play(Sfx::Select); break;
    case kSettingBack:
      if (in.Pressed(Action::Confirm)) { game.Snd().Play(Sfx::Select); return true; }
      break;
  }
  return false;
}

void DrawSettingsMenu(Graphics& g, int cursor, int frame, float x, float y, float w, float hintY) {
  constexpr float kRowH = 40.f, kStep = 46.f;
  const Settings& s = Prefs();
  const float right = x + w - 40.f;  // 값의 오른쪽 끝 (그 밖은 ">" 화살표 자리)
  const float bob = std::floor(std::sin(static_cast<float>(frame) * 0.2f) * 2.f);
  const char* labels[kSettingCount] = {"VOLUME", "SCREEN SHAKE", "JUMP", "LANGUAGE", "BACK"};

  for (int i = 0; i < kSettingCount; ++i) {
    const float ry = y + static_cast<float>(i) * kStep + (i == kSettingBack ? 12.f : 0.f);
    const bool sel = i == cursor;
    // 줄 배경: 선택한 줄만 밝은 테두리
    NotchedRect(g, x, ry, w, kRowH, 2.f, sel ? Hex(0xc58bff) : Hex(0x2e1452));
    NotchedRect(g, x + 2.f, ry + 2.f, w - 4.f, kRowH - 4.f, 2.f, sel ? Hex(0x3a1a66) : Hex(0x1c0a34));
    g.Text(labels[i], x + 18.f, ry + 10.f, 3, sel ? Hex(0xffe27a) : Hex(0xffffff));

    // 값: 바꿀 수 있는 줄은 선택했을 때 좌우 화살표를 보여 준다
    float left = right;  // 값이 시작되는 x (화살표 위치용)
    const float vy = ry + 7.f;
    if (i == kSettingVolume) {
      for (int k = 9; k >= 0; --k) {  // 칸 막대: 채운 칸 = 크기
        left -= 14.f;
        g.FillRect(left, vy, 12.f, 26.f, Hex(0x12061f));
        g.FillRect(left + 2.f, vy + 2.f, 8.f, 22.f, k < s.volume ? Lerp(Hex(0x9a5cff), Hex(0xffe27a), static_cast<float>(k) / 9.f) : Hex(0x2e1452));
      }
    } else if (i == kSettingShake || i == kSettingLanguage) {
      // 고를 수 있는 값을 나란히 놓고 고른 칸만 채운다 (OFF / LOW / FULL, ENGLISH / 한국어)
      const bool shake = i == kSettingShake;
      const int count = shake ? 3 : 2, chosen = shake ? s.shake : static_cast<int>(s.language);
      for (int k = count - 1; k >= 0; --k) {
        const char* name = shake ? ShakeName(k) : LanguageName(k);
        const float sw = static_cast<float>(g.TextWidth(name, 2)) + 16.f;
        left -= sw;
        const bool on = k == chosen;
        NotchedRect(g, left, vy, sw, 26.f, 2.f, on ? Hex(0xffe27a) : Hex(0x12061f));
        if (!on) NotchedRect(g, left + 2.f, vy + 2.f, sw - 4.f, 22.f, 2.f, Hex(0x1c0a34));
        g.Text(name, left + sw * 0.5f, vy + 6.f, 2, on ? Hex(0x2a1048) : Hex(0x8a74b0), Align::Center, false);
        left -= 6.f;
      }
      left += 6.f;
    } else if (i == kSettingJump) {
      const char* name = JumpName(s.jump);
      left -= static_cast<float>(g.TextWidth(name, 3));
      g.Text(name, left, ry + 10.f, 3, Hex(0x7ae0ff));
    }
    if (sel && i != kSettingBack) {
      g.Text("<", left - 22.f - bob, ry + 10.f, 3, Hex(0xffe27a), Align::Center);
      g.Text(">", right + 20.f + bob, ry + 10.f, 3, Hex(0xffe27a), Align::Center);
    }
  }

  // 고른 항목 설명
  const char* hint = cursor == kSettingVolume     ? "SOUND EFFECT VOLUME"
                     : cursor == kSettingShake    ? "HOW MUCH THE SCREEN SHAKES ON BIG HITS"
                     : cursor == kSettingJump     ? JumpHint(s.jump)
                     : cursor == kSettingLanguage ? "TEXT LANGUAGE"
                                                  : "BACK TO THE MENU";
  g.Text(hint, x + w * 0.5f, hintY, 2, Hex(0xc9b0ee), Align::Center, false);
}

}  // namespace gengar

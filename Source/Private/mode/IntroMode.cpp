#include "mode/IntroMode.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "actor/PhantomArt.h"
#include "art/Art.h"
#include "core/Config.h"
#include "core/DebugOptions.h"
#include "core/Locale.h"
#include "core/Game.h"
#include "platform/Audio.h"
#include "platform/Graphics.h"
#include "platform/Input.h"

namespace gengar {
namespace {

constexpr float kW = static_cast<float>(cfg::kScreenW);
constexpr float kH = static_cast<float>(cfg::kScreenH);
constexpr float kGroundY = 410.f;

// ---- 타임라인 (프레임, 60 = 1초) ----
constexpr int kHatShine = 240;     // 모자가 반짝인다
constexpr int kDarken = 400;       // 하늘이 어두워지고 땅이 울린다
constexpr int kBossArrive = 440;   // VOIDMAW 강림 (~560)
constexpr int kReach = 620;        // 손을 뻗는다
constexpr int kGrab = 680;         // 모자를 낚아챈다
constexpr int kHandBack = 740;     // 손이 돌아온다
constexpr int kFlee = 800;         // 모자를 들고 달아난다 (~900)
constexpr int kResolve = 920;      // 팬텀의 다짐
constexpr int kRun = 1060;         // VOIDMAW가 날아간 쪽(오른쪽)으로 달려 나간다
constexpr int kFadeOut = 1120;
constexpr int kEnd = 1180;

const CutsceneBeat kBeats[] = {
    {0, kHatShine, "", "PHANTOM HAD A SPECIAL TREASURE.\nIT LOVED IT SO MUCH."},
    {kHatShine, kDarken, "", "IT WAS A SHINY TREASURE HAT!"},
    {kDarken, 600, "", "BUT ONE NIGHT, A BIG MONSTER\nNAMED VOIDMAW CAME DOWN!"},
    {600, kFlee, "VOIDMAW", "OH, WHAT A SHINY HAT!\nI WILL TAKE IT! HA HA HA!"},
    {kFlee, kResolve, "", "VOIDMAW RAN AWAY WITH THE HAT."},
    {kResolve, kRun, "PHANTOM", "MY HAT!\nI WILL GET IT BACK!"},
};
constexpr int kBeatCount = static_cast<int>(sizeof(kBeats) / sizeof(kBeats[0]));

float Ramp(int t, int a, int b) { return std::clamp(static_cast<float>(t - a) / static_cast<float>(b - a), 0.f, 1.f); }
float EaseOut(float k) { return 1.f - (1.f - k) * (1.f - k); }
float EaseInOut(float k) { return k * k * (3.f - 2.f * k); }

const Vec2 kPhantomPos{300.f, kGroundY - 58.f};
constexpr float kPhantomScale = 2.5f;

// 달려 나가는 팬텀의 x 위치: 점점 빨라지며 화면 오른쪽 밖으로
float RunX(int t) {
  const float k = Ramp(t, kRun, kRun + 60);
  return kPhantomPos.x + 760.f * k * k;
}

Vec2 BossPos(int t) {
  const float arrive = EaseOut(Ramp(t, kBossArrive, 560));
  const float flee = Ramp(t, kFlee, 900);
  const float hover = std::sin(static_cast<float>(t) * 0.04f) * 10.f;
  return {680.f + flee * flee * 520.f, -220.f + arrive * 400.f - flee * flee * 420.f + hover};
}

// 왼손(팬텀 쪽 손)의 위치. 모자를 낚아채러 뻗었다가 돌아온다.
Vec2 GrabHandPos(int t, Vec2 boss, Vec2 hatAnchor) {
  const Vec2 rest{boss.x - 150.f, boss.y + 50.f + std::cos(static_cast<float>(t) * 0.06f) * 8.f};
  const Vec2 grab{hatAnchor.x, hatAnchor.y - 40.f};  // 손에 든 모자 챙이 머리 위 자리와 겹치는 위치
  float k = 0.f;
  if (t >= kReach && t < kGrab) k = EaseInOut(Ramp(t, kReach, kGrab));
  else if (t >= kGrab && t < kHandBack) k = 1.f - EaseInOut(Ramp(t, kGrab + 10, kHandBack));
  return {rest.x + (grab.x - rest.x) * k, rest.y + (grab.y - rest.y) * k};
}

void DrawBossHand(Graphics& g, Vec2 c, bool flip) {
  const SpriteId hand = art::A().bossHand[0];
  const Vec2 hs = g.SpriteSize(hand);
  g.DrawSprite(hand, std::floor(c.x - hs.x * 1.5f), std::floor(c.y - hs.y * 1.5f), 3, flip);
}

}  // namespace

void IntroMode::Update(Game& game) {
  prev_ = t_;
  ++blink_;
  // 대사는 자동으로 넘어가지 않는다: 장면의 마지막 프레임에서 멈춰 Enter/Z를 기다린다.
  const int cur = BeatAt(kBeats, kBeatCount, t_);
  const bool waiting = cur >= 0 && t_ >= kBeats[cur].end - 1;
  if (!waiting) ++t_;
  Input& in = game.In();
  if (skip_.Update(in)) { DebugLog("[intro] skipped at t=%d", t_); game.StartNewRun(); return; }
  if (in.Pressed(Action::Confirm)) {
    const int beat = BeatAt(kBeats, kBeatCount, t_);
    if (beat >= 0) {
      const CutsceneBeat& b = kBeats[beat];
      const int typedAt = b.start + kFramesPerChar * static_cast<int>(CharCount(Localize(b.text)));
      // 글자가 아직 나오는 중이면 먼저 전부 보여 주고, 다 보인 뒤에 누르면 다음 장면으로
      t_ = (t_ < typedAt) ? std::min(typedAt, b.end - 1) : b.end;
    }
    DebugLog("[intro] next -> t=%d", t_);
  }
  if (t_ >= kEnd) { DebugLog("[intro] finished"); game.StartNewRun(); return; }

  Audio& snd = game.Snd();
  if (Crossed(kHatShine)) snd.Play(Sfx::Heart);
  if (Crossed(kDarken)) snd.Play(Sfx::CrumbleShake);
  if (Crossed(kBossArrive)) snd.Play(Sfx::BossRoar);
  if (Crossed(kReach)) snd.Play(Sfx::BossBlink);
  if (Crossed(kGrab)) { snd.Play(Sfx::PlayerHurt); snd.Play(Sfx::Alert); }
  if (Crossed(kFlee)) snd.Play(Sfx::BossDash);
  if (Crossed(kResolve + 60)) snd.Play(Sfx::Jump);
  if (Crossed(kRun)) snd.Tone(260.f, 820.f, 0.15f, Wave::Saw, 0.06f);
  if (t_ > kRun && t_ < kRun + 50 && t_ % 8 == 0) snd.Play(Sfx::CrumbleShake);  // 다다다 발소리
  // 타자기 소리
  const int beat = BeatAt(kBeats, kBeatCount, t_);
  if (beat >= 0 && t_ % 4 == 0) {
    const int typed = (t_ - kBeats[beat].start) / 2;
    if (typed < static_cast<int>(CharCount(Localize(kBeats[beat].text)))) snd.Tone(900.f, 900.f, 0.025f, Wave::Square, 0.02f);
  }
}

void IntroMode::Render(Graphics& g) {
  const int t = t_;
  const float ft = static_cast<float>(t);

  // 하늘: 평화로운 밤 -> 공허의 기운으로 어두워짐 (VOIDMAW가 떠나면 조금 걷힌다)
  const float dark = Ramp(t, kDarken, kDarken + 60) * (1.f - 0.6f * Ramp(t, kFlee + 40, kResolve + 60));
  DrawNightSky(g, t, Lerp(Hex(0x0e0420), Hex(0x05010c), dark), Lerp(Hex(0x3a1260), Hex(0x4a0a2a), dark));
  g.DrawSprite(art::A().moon, 90.f, 50.f, 3, false, 1.f - dark * 0.8f);

  // 땅울림
  const bool rumble = t >= kDarken && t < 570;
  g.SetOffset(rumble ? std::sin(ft * 1.7f) * 3.f : 0.f, rumble ? std::cos(ft * 2.3f) * 2.f : 0.f);

  // 언덕
  g.FillRect(-8, kGroundY, kW + 16, kH - kGroundY, Hex(0x1a0c2e));
  g.FillRect(-8, kGroundY, kW + 16, 6, Hex(0x4a2480));
  for (int i = 0; i < 24; ++i) g.FillRect(static_cast<float>((i * 97) % cfg::kScreenW), kGroundY - 4.f, 4, 4, Hex(0x4a2480));

  // VOIDMAW
  const Vec2 boss = BossPos(t);
  const float bossAlpha = Ramp(t, kBossArrive, kBossArrive + 50);
  if (t >= kBossArrive) {
    const art::Assets& a = art::A();
    g.Glow(boss.x, boss.y, 170.f, Rgb(180, 60, 255, static_cast<int>(110.f * bossAlpha)));
    g.SetGlobalAlpha(bossAlpha);
    const Vec2 bs = g.SpriteSize(a.boss[0]);
    g.DrawSprite(a.boss[0], std::floor(boss.x - bs.x * 1.5f), std::floor(boss.y - bs.y * 1.5f), 3);
    g.FillRect(std::floor(boss.x - 12.f), std::floor(boss.y - 18.f), 9, 27, Hex(0x12000a));  // 팬텀 쪽을 노려보는 눈동자
    DrawBossHand(g, {boss.x + 150.f, boss.y + 50.f + std::cos(ft * 0.06f + 1.f) * 8.f}, false);
    g.SetGlobalAlpha(1.f);
  }

  // 팬텀: 모자를 빼앗기기 전에는 행복한 얼굴
  const bool hatOnHead = t < kGrab;
  const bool shocked = t >= kGrab && t < kGrab + 70;
  Vec2 pp = kPhantomPos;
  if (shocked) {
    pp.x += std::sin(ft * 2.1f) * 3.f;
    pp.y -= std::sin(Ramp(t, kGrab, kGrab + 24) * kPi) * 30.f;
  }
  if (t >= kResolve + 60 && t < kResolve + 84) pp.y -= std::sin(Ramp(t, kResolve + 60, kResolve + 84) * kPi) * 24.f;  // 다짐의 점프
  const bool running = t >= kRun;
  if (running) {
    pp.x = RunX(t);
    // 발밑 먼지: 지나간 자리에 남은 퍼프가 위로 흩어진다
    for (int i = 0; i < 6; ++i) {
      const int age = t - kRun - i * 7;
      if (age < 0 || age >= 20) continue;
      const float x = RunX(t - age) - 40.f, s = std::floor((1.f - static_cast<float>(age) / 20.f) * 5.f) * 2.f + 2.f;
      g.FillRect(x - s * 0.5f, kGroundY - 6.f - static_cast<float>(age), s, s, Rgb(214, 198, 240, 200));
    }
  }
  PhantomStyle style;
  style.happy = t < kDarken;
  style.hat = hatOnHead;
  style.hatGlow = (t >= kHatShine && t < kDarken) ? 1.f : 0.f;
  if (running) style.squash = {1.2f, 0.88f};  // 앞으로 몸을 숙이고 달린다
  DrawPhantom(g, pp, 1, running ? ft * 3.f : ft, kPhantomScale, 1.f, style);
  if (shocked || (t >= kGrab && t < kFlee && (t / 10) % 2 == 0)) g.Text("!", pp.x + 34.f, pp.y - 96.f, 6, Hex(0xffe27a), Align::Center);
  if (t >= kResolve && t < kResolve + 60) g.Text("...", pp.x + 40.f, pp.y - 90.f, 4, Hex(0xc9b0ee), Align::Center);

  // 낚아채는 손 + 손에 들린 모자
  if (t >= kBossArrive && t < 920) {
    const Vec2 hand = GrabHandPos(t, boss, PhantomHatAnchor(kPhantomPos, 1, ft, kPhantomScale));
    g.SetGlobalAlpha(bossAlpha);
    if (!hatOnHead) DrawTreasureHat(g, {hand.x, hand.y + 40.f}, kPhantomScale, 0.3f, ft);
    DrawBossHand(g, hand, true);
    g.SetGlobalAlpha(1.f);
  }
  if (t >= kGrab && t < kGrab + 8) g.FillRect(0, 0, kW, kH, Rgb(255, 255, 255, 120));  // 낚아채는 순간 번쩍

  g.SetOffset(0.f, 0.f);
  DrawLetterbox(g, Ramp(t, 0, 40));
  const int beat = BeatAt(kBeats, kBeatCount, t);
  if (beat >= 0) DrawDialog(g, kBeats[beat], t, blink_);
  skip_.Render(g);

  // 페이드 인/아웃
  const float fade = std::max(1.f - Ramp(t, 0, 30), Ramp(t, kFadeOut, kEnd));
  if (fade > 0.f) g.FillRect(0, 0, kW, kH, Rgb(0, 0, 0, static_cast<int>(fade * 255.f)));
}

}  // namespace gengar

#include "platform/Audio.h"

#include <SDL.h>

#include <algorithm>
#include <cmath>

#include "core/Types.h"

namespace gengar {
namespace {

float Osc(Wave w, float p) {
  switch (w) {
    case Wave::Square: return p < 0.5f ? 1.f : -1.f;
    case Wave::Triangle: return 4.f * std::fabs(p - 0.5f) - 1.f;
    case Wave::Sine: return std::sin(2.f * kPi * p);
    case Wave::Saw: return 2.f * p - 1.f;
  }
  return 0.f;
}

}  // namespace

Audio::~Audio() { Shutdown(); }

bool Audio::Init() {
  SDL_AudioSpec want{}, have{};
  want.freq = 44100;
  want.format = AUDIO_F32SYS;
  want.channels = 1;
  want.samples = 1024;
  want.callback = &Audio::Callback;
  want.userdata = this;
  device_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
  if (device_ == 0) return false;
  rate_ = static_cast<float>(have.freq);
  voices_.reserve(24);
  SDL_PauseAudioDevice(device_, 0);
  return true;
}

void Audio::Shutdown() {
  if (device_ != 0) {
    SDL_CloseAudioDevice(device_);
    device_ = 0;
  }
}

void Audio::Callback(void* userdata, uint8_t* stream, int len) {
  static_cast<Audio*>(userdata)->Mix(reinterpret_cast<float*>(stream), len / static_cast<int>(sizeof(float)));
}

void Audio::Mix(float* out, int n) {
  for (int i = 0; i < n; ++i) out[i] = 0.f;
  for (Voice& v : voices_) {
    if (!v.active) continue;
    for (int i = 0; i < n; ++i) {
      const float p = v.t / v.dur;
      if (p >= 1.f) { v.active = false; break; }
      const float f = v.f0 * std::pow(v.f1 / v.f0, p);
      v.phase += f / rate_;
      if (v.phase >= 1.f) v.phase -= 1.f;
      const float env = (1.f - p) * (1.f - p);
      out[i] += Osc(v.wave, v.phase) * v.vol * env;
      v.t += 1.f / rate_;
    }
  }
  for (int i = 0; i < n; ++i) out[i] = Clamp(out[i] * volume_, -1.f, 1.f);
}

void Audio::Tone(float f0, float f1, float dur, Wave wave, float volume) {
  if (device_ == 0 || dur <= 0.f || f0 <= 0.f) return;
  if (f1 <= 0.f) f1 = f0;
  SDL_LockAudioDevice(device_);
  Voice* slot = nullptr;
  for (Voice& v : voices_) {
    if (!v.active) { slot = &v; break; }
  }
  if (!slot && voices_.size() < 24) {
    voices_.emplace_back();
    slot = &voices_.back();
  }
  if (slot) *slot = Voice{f0, f1, dur, volume, 0.f, 0.f, wave, true};
  SDL_UnlockAudioDevice(device_);
}

void Audio::Play(Sfx sfx) {
  switch (sfx) {
    case Sfx::Jump: Tone(330, 630, 0.18f, Wave::Sine, 0.05f); break;
    case Sfx::Shoot: Tone(520, 220, 0.14f, Wave::Sine, 0.06f); break;
    case Sfx::EnemyHit: Tone(240, 140, 0.08f, Wave::Square, 0.05f); break;
    case Sfx::EnemyDie: Tone(700, 1100, 0.15f, Wave::Triangle, 0.05f); break;
    case Sfx::Coin: Tone(980, 1280, 0.10f, Wave::Square, 0.04f); break;
    case Sfx::Heart: Tone(600, 1100, 0.25f, Wave::Triangle, 0.06f); break;
    case Sfx::PlayerHurt: Tone(180, 60, 0.25f, Wave::Saw, 0.07f); break;
    case Sfx::PlayerDie: Tone(300, 50, 0.6f, Wave::Saw, 0.07f); break;
    case Sfx::Ultimate:
      Tone(70, 40, 0.7f, Wave::Saw, 0.09f);
      Tone(400, 1100, 0.5f, Wave::Triangle, 0.06f);
      break;
    case Sfx::BossHit: Tone(200, 120, 0.08f, Wave::Saw, 0.05f); break;
    case Sfx::BossShoot: Tone(150, 350, 0.3f, Wave::Saw, 0.06f); break;
    case Sfx::BossRoar: Tone(130, 280, 0.4f, Wave::Saw, 0.07f); break;
    case Sfx::BossSlam: Tone(70, 40, 0.5f, Wave::Saw, 0.09f); break;
    case Sfx::BossPhase: Tone(100, 160, 0.8f, Wave::Saw, 0.08f); break;
    case Sfx::BossDeath: Tone(90, 40, 1.2f, Wave::Saw, 0.08f); break;
    case Sfx::BossSummon: Tone(400, 150, 0.4f, Wave::Triangle, 0.06f); break;
    case Sfx::BossBlink: Tone(500, 200, 0.3f, Wave::Sine, 0.06f); break;
    case Sfx::BossDash: Tone(110, 310, 0.5f, Wave::Saw, 0.08f); break;
    case Sfx::BeamCharge: Tone(300, 900, 0.5f, Wave::Sine, 0.05f); break;
    case Sfx::BeamFire: Tone(90, 210, 0.6f, Wave::Saw, 0.08f); break;
    case Sfx::PillarWarn: Tone(120, 220, 0.4f, Wave::Saw, 0.06f); break;
    case Sfx::PillarHit: Tone(80, 55, 0.25f, Wave::Saw, 0.07f); break;
    case Sfx::Alert: Tone(160, 80, 0.2f, Wave::Saw, 0.05f); break;
    case Sfx::CrumbleShake: Tone(200, 140, 0.1f, Wave::Square, 0.04f); break;
    case Sfx::CrumbleFall: Tone(110, 60, 0.3f, Wave::Saw, 0.05f); break;
    case Sfx::Select: Tone(500, 500, 0.05f, Wave::Square, 0.03f); break;
    case Sfx::Buy: Tone(880, 1480, 0.25f, Wave::Triangle, 0.07f); break;
    case Sfx::Error: Tone(130, 90, 0.2f, Wave::Saw, 0.05f); break;
    case Sfx::StageClear: Tone(660, 1260, 0.5f, Wave::Triangle, 0.07f); break;
  }
}

}  // namespace gengar

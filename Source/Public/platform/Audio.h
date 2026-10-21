#pragma once
// 파일 없이 코드로 합성하는 효과음. SDL 오디오 콜백에서 보이스들을 믹싱한다.
#include <cstdint>
#include <vector>

namespace gengar {

enum class Wave { Square, Triangle, Sine, Saw };

enum class Sfx {
  Jump, Shoot, EnemyHit, EnemyDie, Coin, Heart, PlayerHurt, PlayerDie, Ultimate,
  BossHit, BossShoot, BossRoar, BossSlam, BossPhase, BossDeath, BossSummon, BossBlink, BossDash,
  BeamCharge, BeamFire, PillarWarn, PillarHit, Alert, CrumbleShake, CrumbleFall,
  Select, Buy, Error, StageClear,
};

class Audio {
 public:
  Audio() = default;
  ~Audio();
  Audio(const Audio&) = delete;
  Audio& operator=(const Audio&) = delete;

  bool Init();
  void Shutdown();

  void Play(Sfx sfx);
  // f0에서 f1까지 지수 슬라이드하며 dur초 동안 재생
  void Tone(float f0, float f1, float dur, Wave wave, float volume);

 private:
  struct Voice {
    float f0 = 0, f1 = 0, dur = 0, vol = 0, t = 0, phase = 0;
    Wave wave = Wave::Square;
    bool active = false;
  };
  static void Callback(void* userdata, uint8_t* stream, int len);
  void Mix(float* out, int n);

  uint32_t device_ = 0;  // SDL_AudioDeviceID
  float rate_ = 44100.f;
  std::vector<Voice> voices_;
};

}  // namespace gengar

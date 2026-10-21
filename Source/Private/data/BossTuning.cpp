#include "data/BossTuning.h"

namespace gengar {
namespace {

// 단계별 값을 "1단계, 2단계, 3단계" 순서로 적는다.
template <class T>
T Pick(int phase, T p0, T p1, T p2) { return phase <= 0 ? p0 : phase == 1 ? p1 : p2; }

BossTuning Make(int p) {
  BossTuning t;
  t.moveSpeed = Pick(p, 3.0f, 3.9f, 5.0f);
  t.idleFrames = Pick(p, 55, 42, 30);

  t.volleyCount = Pick(p, 4, 5, 7);
  t.volleyRepeats = Pick(p, 2, 2, 3);
  t.volleySpeed = Pick(p, 5.0f, 5.7f, 6.4f);
  t.ringCount = Pick(p, 16, 18, 20);
  t.ringWaves = Pick(p, 2, 3, 3);
  t.ringSpeed = Pick(p, 3.2f, 3.5f, 3.8f);
  t.spiralArms = Pick(p, 3, 4, 5);
  t.spiralInterval = Pick(p, 5, 4, 4);
  t.spiralFrames = Pick(p, 110, 120, 130);
  t.spiralSpeed = Pick(p, 3.6f, 4.0f, 4.4f);
  t.homingCount = Pick(p, 3, 4, 5);
  t.homingWaves = Pick(p, 1, 2, 2);
  t.homingSpeed = Pick(p, 3.2f, 3.5f, 3.8f);
  t.homingTurn = Pick(p, 0.028f, 0.032f, 0.036f);
  t.rainInterval = Pick(p, 6, 5, 4);
  t.rainFrames = Pick(p, 110, 120, 130);
  t.rainSpeed = Pick(p, 6.5f, 7.2f, 7.9f);
  t.pillarCount = Pick(p, 3, 4, 5);
  t.pillarWaves = Pick(p, 2, 2, 3);
  t.pillarWarn = Pick(p, 40, 34, 28);
  t.beamFireAt = Pick(p, 55, 48, 42);
  t.beamSweep = Pick(p, 0.0f, 0.010f, 0.022f);
  t.blinkWaves = Pick(p, 2, 3, 3);
  t.blinkCount = Pick(p, 5, 7, 7);
  t.blinkSpeed = Pick(p, 5.2f, 5.8f, 6.4f);

  t.lungeWindup = Pick(p, 55, 46, 38);
  t.lungeExtra = Pick(p, 0, 1, 2);
  t.lungeRest = Pick(p, 40, 34, 28);
  t.lungeSpeed = Pick(p, 13.0f, 15.0f, 17.0f);
  t.dashWindup = Pick(p, 48, 40, 34);
  t.dashRest = Pick(p, 38, 32, 26);
  t.dashSpeed = Pick(p, 11.0f, 13.5f, 16.0f);
  t.slamWindup = Pick(p, 42, 36, 30);
  t.slamRest = Pick(p, 45, 38, 32);
  t.slamWaveSpeed = Pick(p, 5.2f, 6.0f, 6.8f);
  return t;
}

}  // namespace

const BossTuning& Tuning(int phase) {
  static const BossTuning table[3] = {Make(0), Make(1), Make(2)};
  return table[phase < 0 ? 0 : phase > 2 ? 2 : phase];
}

}  // namespace gengar

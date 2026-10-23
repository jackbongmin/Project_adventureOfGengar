#include "data/BossTuning.h"

namespace gengar {
namespace {

// 단계별 값을 "1단계, 2단계, 3단계" 순서로 적는다.
template <class T>
T Pick(int phase, T p0, T p1, T p2) { return phase <= 0 ? p0 : phase == 1 ? p1 : p2; }

BossTuning Make(int p) {
  BossTuning t;
  t.moveSpeed = Pick(p, 3.0f, 3.9f, 5.0f);
  t.idleFrames = Pick(p, 60, 48, 36);

  t.volleyCount = Pick(p, 4, 5, 5);
  t.volleyRepeats = Pick(p, 2, 2, 3);
  t.volleySpeed = Pick(p, 5.0f, 5.7f, 6.4f);
  t.ringCount = Pick(p, 10, 12, 14);
  t.ringWaves = Pick(p, 2, 2, 3);
  t.ringSpeed = Pick(p, 3.2f, 3.5f, 3.8f);
  t.spiralArms = Pick(p, 2, 3, 3);
  t.spiralInterval = Pick(p, 9, 8, 7);
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
  t.pillarWarn = Pick(p, 50, 43, 35);
  t.beamFireAt = Pick(p, 68, 60, 52);
  t.beamSweep = Pick(p, 0.0f, 0.010f, 0.022f);
  t.blinkWaves = Pick(p, 2, 2, 3);
  t.blinkCount = Pick(p, 3, 5, 5);
  t.blinkSpeed = Pick(p, 5.2f, 5.8f, 6.4f);

  t.lungeWindup = Pick(p, 58, 48, 40);
  t.lungeExtra = Pick(p, 1, 2, 3);
  t.lungeRest = Pick(p, 40, 34, 28);
  t.lungeSpeed = Pick(p, 14.0f, 16.0f, 18.0f);
  t.dashWindup = Pick(p, 40, 33, 28);
  t.dashRest = Pick(p, 38, 32, 26);
  t.dashSpeed = Pick(p, 12.0f, 14.5f, 17.0f);
  t.slamWindup = Pick(p, 34, 29, 24);
  t.slamRest = Pick(p, 45, 38, 32);
  t.slamWaveSpeed = Pick(p, 5.6f, 6.4f, 7.2f);

  t.crossRows = Pick(p, 5, 6, 7);
  t.crossWaves = Pick(p, 3, 4, 5);
  t.crossSpeed = Pick(p, 4.4f, 5.0f, 5.6f);
  t.sweepFrames = Pick(p, 100, 120, 140);
  t.sweepInterval = Pick(p, 10, 9, 8);
  t.sweepCount = Pick(p, 2, 3, 3);
  t.sweepSpeed = Pick(p, 4.4f, 4.9f, 5.4f);
  t.sweepArc = Pick(p, 0.9f, 1.1f, 1.3f);
  return t;
}

}  // namespace

const BossTuning& Tuning(int phase) {
  static const BossTuning table[3] = {Make(0), Make(1), Make(2)};
  return table[phase < 0 ? 0 : phase > 2 ? 2 : phase];
}

}  // namespace gengar

#pragma once
// 보스 난이도 표. 단계(phase 0~2)별 속도, 쉬는 시간, 패턴별 탄 수/속도/예고 시간을 한 곳에서 조정한다.
// 값은 BossTuning.cpp 의 표에서 "1단계 / 2단계 / 3단계" 순으로 적혀 있다.

namespace gengar {

struct BossTuning {
  // 공통
  float moveSpeed = 0;   // 떠다니는 이동 속도(px/프레임)
  int idleFrames = 0;    // 공격과 공격 사이 쉬는 시간

  // 원거리
  int volleyCount = 0, volleyRepeats = 0;             // 암흑탄: 한 번에 쏘는 수 / 연속 횟수
  float volleySpeed = 0;
  int ringCount = 0, ringWaves = 0;                   // 공허의 고리: 한 겹의 탄 수 / 겹 수
  float ringSpeed = 0;
  int spiralArms = 0, spiralInterval = 0, spiralFrames = 0;  // 소용돌이: 줄기 수 / 발사 간격 / 지속 시간
  float spiralSpeed = 0;
  int homingCount = 0, homingWaves = 0;               // 유도탄: 한 번에 수 / 연속 횟수
  float homingSpeed = 0, homingTurn = 0;
  int rainInterval = 0, rainFrames = 0;               // 어둠의 비: 낙하 간격 / 지속 시간
  float rainSpeed = 0;
  int pillarCount = 0, pillarWaves = 0, pillarWarn = 0;      // 그림자 기둥: 수 / 연속 횟수 / 예고 시간
  int beamFireAt = 0;                                  // 광선: 조준선이 뜬 뒤 발사까지 걸리는 시간(프레임)
  float beamSweep = 0;                                 // 광선 회전 속도(rad/프레임), 0이면 고정
  int blinkWaves = 0, blinkCount = 0;                  // 순간이동: 사격 횟수 / 한 번에 쏘는 수
  float blinkSpeed = 0;

  // 몸으로 덮치는 기술
  int lungeWindup = 0, lungeExtra = 0, lungeRest = 0;  // 돌격: 예고 시간 / 추가 연속 횟수 / 끝난 뒤 빈틈
  float lungeSpeed = 0;
  int dashWindup = 0, dashRest = 0;                    // 바닥 질주: 준비 시간 / 빈틈
  float dashSpeed = 0;
  int slamWindup = 0, slamRest = 0;                    // 내려찍기: 준비 시간 / 빈틈
  float slamWaveSpeed = 0;

  // 새 원거리 패턴
  int crossRows = 0, crossWaves = 0;                   // 십자포화: 한 번에 날아오는 줄 수 / 연속 횟수 (양쪽 벽에서 엇갈려 발사)
  float crossSpeed = 0;
  int sweepFrames = 0, sweepInterval = 0, sweepCount = 0;  // 허공 휩쓸기: 지속 시간 / 발사 간격 / 한 번에 쏘는 수
  float sweepSpeed = 0, sweepArc = 0;                  // 탄 속도 / 좌우로 훑는 각도 폭(rad)
};

const BossTuning& Tuning(int phase);  // phase 0~2

}  // namespace gengar

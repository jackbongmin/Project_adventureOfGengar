#pragma once
// 개발/검증용 옵션. 웹: 주소 뒤에 ?stage=2&bot=1&god=1&log=1&speed=4&coins=100
//                  네이티브: --stage=2 --bot --god --log --speed=4 --coins=100
//   stage : 시작 스테이지(0~2)      coins : 시작 코인
//   bot   : 자동 플레이 봇이 조종    god   : 피격 무적(낙사는 여전히 사망)
//   speed : 배속(로직 스텝 배율)     log   : 주요 이벤트를 콘솔에 출력
//   bosshp / bossphase(0~2) / immortal : 보스전 검증용 (체력 지정, 시작 단계, 불사)
//   shop  : 시작하자마자 상점 화면   heartdemo / hp=N : 하트 규칙 검증용   heightdemo : 늑대 추적 포기 검증용
namespace gengar {

struct DebugOptions {
  int startStage = 0;
  int startCoins = 0;
  int timeScale = 1;
  bool god = false;
  bool bot = false;
  bool log = false;
  int bossHp = 0;      // >0 이면 보스 최대 체력 지정
  int bossPhase = 0;   // 0~2: 해당 단계의 체력 비율에서 시작
  bool immortal = false;  // 보스가 피해를 받지 않음 (패턴 검증용)
  bool startInShop = false;  // 시작하자마자 상점 화면 (상점 UI 검증용)
  bool heartDemo = false;    // 시작 지점 근처에 하트를 하나 추가 (하트 규칙 검증용)
  bool heightDemo = false;   // 플레이어를 위 발판에 올리고 아래에 추적 중인 늑대 배치 (추적 포기 검증용)
  int startHp = 0;           // >0 이면 플레이어 시작 체력
};

DebugOptions& Debug();
void ParseDebugOptions(int argc, char** argv);
void DebugLog(const char* fmt, ...);  // log=1 일 때만 출력

}  // namespace gengar

#pragma once
// 시네마틱(오프닝/엔딩) 공용 도구: 대사 상자, 밤하늘 배경, "C를 꾹 눌러 건너뛰기".
// 시네마틱은 모두 "시작 후 경과 프레임 t"만으로 그려서, 어느 장면으로 건너뛰어도 화면이 깨지지 않는다.
#include <string>

#include "core/Types.h"

namespace gengar {

class Graphics;
class Input;

constexpr int kFramesPerChar = 2;  // 타자기 속도: 글자 하나가 나타나는 데 걸리는 프레임

// 장면(비트) 하나 = 구간 + 대사. speaker가 비어 있으면 내레이션.
struct CutsceneBeat {
  int start;
  int end;
  const char* speaker;
  const char* text;
};

// C를 약 1초 꾹 누르면 true를 한 번 돌려준다. 떼면 게이지가 빠르게 줄어든다.
class SkipHold {
 public:
  static constexpr int kHoldFrames = 60;
  bool Update(const Input& in);
  void Render(Graphics& g) const;

 private:
  int held_ = 0;
  bool fired_ = false;
};

// t 프레임에 해당하는 장면 번호 (없으면 -1)
int BeatAt(const CutsceneBeat* beats, int count, int t);
// 화면 아래 대사 상자. 글자는 한 글자씩(타자기) 나타난다.
// blinkT: "다음" 화살표 깜빡임에 쓰는 프레임. 장면이 멈춰 있어도 깜빡이게 t와 따로 줄 수 있다 (-1이면 t).
void DrawDialog(Graphics& g, const CutsceneBeat& beat, int t, int blinkT = -1);
// 위아래 검은 띠 (영화 느낌)
void DrawLetterbox(Graphics& g, float amount01);
void DrawNightSky(Graphics& g, int frame, Color top, Color bottom);

}  // namespace gengar

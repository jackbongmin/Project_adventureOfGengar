#pragma once
// 게임 버전. 타이틀/일시정지 화면 오른쪽 아래와 네이티브 창 제목에 표시한다.
//  MAJOR.MINOR.PATCH
//  - MAJOR: 정식 출시 전에는 0. 첫 정식 출시 때 1.0.0
//  - MINOR: 스테이지, 적, 메뉴 같은 기능이 늘어날 때 올리고 PATCH 는 0으로
//  - PATCH: 기존 기능의 버그 수정, 밸런스 조정만 있을 때 올린다

namespace gengar::version {

inline constexpr const char* kNumber = "0.2.2";
inline constexpr const char* kLabel = "v0.2.2 BETA";

}  // namespace gengar::version

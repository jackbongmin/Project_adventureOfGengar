#pragma once
// 캐릭터 패널: 레벨/경험치, 능력치(아이콘 + 숫자), 장착 필살기, 얻은 특수 카드 아이콘 격자.
// 상점 왼쪽과 일시정지 화면에서 쓴다.
namespace gengar {

class Graphics;
struct Session;

void DrawCharacterPanel(Graphics& g, const Session& s, float x, float y, float w, float h, int frame);

}  // namespace gengar

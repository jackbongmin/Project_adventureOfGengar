#pragma once
// 플레이 중 화면 상단 HUD (체력, 목숨, 코인, 필살기 게이지, 점수, 보스 체력바, 배너).

namespace gengar {

class Graphics;
class World;
struct Session;

void DrawHud(Graphics& gfx, const World& world, const Session& session);

}  // namespace gengar

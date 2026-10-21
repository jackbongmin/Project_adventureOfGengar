#pragma once
// World/Actor가 접근하는 공유 서비스 묶음. (전역 변수 대신 이 구조체를 전달한다.)

namespace gengar {

class Input;
class Audio;
struct Session;

struct GameContext {
  Input& input;
  Audio& audio;
  Session& session;
};

}  // namespace gengar

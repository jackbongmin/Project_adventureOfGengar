#pragma once
// 컨트롤러 인터페이스: "누가 조종하는가"를 액터에서 분리한다.
// PlayerController(키보드)와 AI 컨트롤러(순찰/추적/포탑/비행)가 같은 인터페이스를 구현한다.
#include "controller/Intent.h"

namespace gengar {

class Character;
class World;

class Controller {
 public:
  virtual ~Controller() = default;
  virtual void Think(Character& self, World& world, Intent& out) = 0;
};

}  // namespace gengar

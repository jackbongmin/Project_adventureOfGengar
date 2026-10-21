#include <SDL.h>  // Windows에서 SDL2main 이 main 을 SDL_main 으로 바꿔준다.

#include "core/Game.h"

int main(int /*argc*/, char** /*argv*/) {
  gengar::Game game;
  if (!game.Init()) return 1;
  game.Run();
  return 0;
}

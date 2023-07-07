#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>

#include <cstdio>
#include <iostream>

#include "../include/core.h"
#include "../include/renderEngine.h"

renderEngine *engine = nullptr;
Uint32 frameStart;
int currentTickTime;

int main(int argc, char *args[]) {
  engine = new renderEngine();

  engine->Initialise("Fluidised Bed Engine", 0, 0, 800, 640, false);

  while (engine->Running()) {
    frameStart = SDL_GetTicks();
    engine->Update();
    engine->Render();

    currentTickTime = SDL_GetTicks() - frameStart;
    if (RE_TICKRATE_TIME > currentTickTime) {
      SDL_Delay(RE_TICKRATE_TIME - currentTickTime);
    } else {
      std::cout << "Tickrate lagging: ";
      std::cout << (currentTickTime - RE_TICKRATE_TIME);
      std::cout << "ms behind!" << std::endl;
    }
  }

  engine->Clean();
  return 0;
}
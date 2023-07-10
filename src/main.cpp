#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>

#include <cstdio>
#include <iostream>

#include "../include/core.h"
#include "../include/fluidEngine.h"
#include "../include/renderEngine.h"

renderEngine *engine = nullptr;
fluidEngine *fengine = nullptr;
Uint32 frameStart;
int currentTickTime;

int main(int argc, char *args[]) {
  engine = new renderEngine();
  fengine = new fluidEngine();

  engine->Initialise("Fluidised Bed Engine", 1280, 720);
  fengine->Start(engine);

  engine->Start(&fengine->cfg_gravity, &fengine->cfg_dampen,
                &fengine->cfg_size);

  if (FB_MOLECULE_SPAWNRANDOM) {
    for (int i = 0; i < FB_MOLECULE_COUNT; i++) {
      int x, y;
      x = (rand() % FB_SIZE);
      y = (rand() % FB_SIZE);
      fengine->AddSandAtPos(x, y);
    }
  } else {
    for (int x = 0; x < FB_MOLECULE_COUNT; x++) {
      for (int y = 0; y < FB_MOLECULE_COUNT; y++) {
        fengine->AddSandAtPos(x, y);
      }
    }
  }

  while (engine->Running()) {
    frameStart = SDL_GetTicks();
    fengine->Update();
    float pixels[FB_SIZE * FB_SIZE * 3];
    float *test = fengine->SandToColour(pixels);
    engine->UpdateImage(test);
    engine->Update();
    engine->Render();

    currentTickTime = SDL_GetTicks() - frameStart;
    if (FB_TICKRATE_TIME > currentTickTime) {
      SDL_Delay(FB_TICKRATE_TIME - currentTickTime);
    } else {
      std::cout << "Tickrate lagging: ";
      std::cout << (currentTickTime - FB_TICKRATE_TIME);
      std::cout << "ms behind!" << std::endl;
    }
  }

  engine->Clean();
  return 0;
}
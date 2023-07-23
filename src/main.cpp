#include <SDL.h>

#include <cstdio>
#include <iostream>

#include "../include/core.h"
#include "../include/fluidEngine.h"
#include "../include/renderEngine.h"

renderEngine *engine = nullptr;
fluidEngine *fengine = nullptr;

Uint32 frameStart;
int currentTickTime;
float pixels[FB_CONTAINER_OUTPUT * FB_CONTAINER_OUTPUT * 3];

// Entrypoint
int main(int argc, char *args[]) {
  // Engines
  engine = new renderEngine();
  fengine = new fluidEngine();

  // Start
  engine->Initialise("Fluidised Bed Engine", 1280, 720);
  fengine->Start(engine);
  engine->LinkSettings(&fengine->settings);

  // Spawn initial random sand
  if (FB_MOLECULE_SPAWNRANDOM) {
    for (int i = 0; i < FB_MOLECULE_COUNT; i++) {
      fengine->AddSandAtRnd();
    }
  } else {
    for (int x = 0; x < FB_MOLECULE_COUNT; x++) {
      for (int y = 0; y < FB_MOLECULE_COUNT; y++) {
        fengine->AddSandAtPos(x, y);
      }
    }
  }

  // Tick loop
  while (engine->Running()) {
    // Start tick time
    frameStart = SDL_GetTicks();

    // Update & render
    fengine->Update();
    fengine->SandToColour(&pixels[0]);
    engine->UpdateImage(&pixels[0]);  // HERE
    engine->val_totalSand = fengine->SandCount();
    engine->Update();
    engine->Render();

    // Check for delays
    currentTickTime = SDL_GetTicks() - frameStart;
    if (FB_TICKRATE_TIME > currentTickTime) {
      SDL_Delay(FB_TICKRATE_TIME - currentTickTime);
    } else {
      std::cout << "Tickrate lagging: ";
      std::cout << (currentTickTime - FB_TICKRATE_TIME);
      std::cout << "ms behind!" << std::endl;
    }
  }
  // Clean
  engine->Clean();
  return 0;
}
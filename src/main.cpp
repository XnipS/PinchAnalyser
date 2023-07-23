#include <SDL.h>

#include <cstdio>
#include <iostream>
#include <thread>

#include "../include/core.h"
#include "../include/fluidEngine.h"
#include "../include/renderEngine.h"

renderEngine *render = nullptr;
fluidEngine *fluid = nullptr;

Uint32 frameStart;
int currentTickTime;
float pixels[FB_CONTAINER_OUTPUT * FB_CONTAINER_OUTPUT * 3];

// Entrypoint
int main(int argc, char *args[]) {
  // Engines
  render = new renderEngine();
  fluid = new fluidEngine();

  // Start
  render->Initialise("Fluidised Bed Engine", 1280, 720);
  fluid->Start(render);
  render->LinkSettings(&fluid->settings);

  // Spawn initial random sand
  if (FB_MOLECULE_SPAWNRANDOM) {
    for (int i = 0; i < FB_MOLECULE_COUNT; i++) {
      fluid->AddSandAtRnd();
    }
  } else {
    for (int x = 0; x < FB_MOLECULE_COUNT; x++) {
      for (int y = 0; y < FB_MOLECULE_COUNT; y++) {
        fluid->AddSandAtPos(x, y);
      }
    }
  }

  // Tick loop
  while (render->Running()) {
    // Start tick time
    frameStart = SDL_GetTicks();

    // Update & render
    std::thread fluidThread(&fluidEngine::Update, fluid);
    fluid->SandToColour(&pixels[0]);
    render->UpdateImage(&pixels[0]);  // HERE
    render->val_totalSand = fluid->SandCount();
    render->Update();
    render->Render();

    fluidThread.join();

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
  render->Clean();
  return 0;
}
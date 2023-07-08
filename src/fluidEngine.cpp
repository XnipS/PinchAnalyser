#include "../include/fluidEngine.h"

#include <cstddef>
#include <cstdio>

fluidEngine::fluidEngine(){};
fluidEngine::~fluidEngine(){};
void fluidEngine::AddSandAtPos(int x, int y) { sand[x][y] = true; };
void fluidEngine::Start() {
  for (int x = 0; x < 500; x++) {
    for (int y = 0; y < 500; y++) {
      sand[x][y] = false;
    }
  }
  printf("Fluid Initialised\n");
};
void fluidEngine::Update() {
  for (int x = 0; x < 500; x++) {
    for (int y = 0; y < 500; y++) {
      if (y != 0 && y != 499) {
        if (sand[x][y]) {
          sand[x][y] = false;
          sand[x][y - 1] = true;
        }
      }
    }
  }
};

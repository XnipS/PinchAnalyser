#include "../include/fluidEngine.h"

#include <cstddef>
#include <cstdio>

#include "../include/renderEngine.h"

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
      if (y != 0) {
        if (sand[499 - x][499 - y]) {
          // printf("move!");
          sand[499 - x][499 - y] = false;
          sand[499 - x][499 - y + 1] = true;
        }
      }
    }
  }
};
float* fluidEngine::SandToColour(float colours[]) {
  int w = 500, h = 500;
  for (int x = 0; x < (w); x++) {
    for (int y = 0; y < (h); y++) {
      Colour3 col;
      if (sand[x][y]) {
        col.r = 1;
        col.g = 1;
        col.b = 1;
      } else {
        col.r = .2;
        col.g = .2;
        col.b = .2;
      }
      colours[(y * w * 3) + (x * 3)] = col.r;
      colours[(y * w * 3) + (x * 3) + 1] = col.b;
      colours[(y * w * 3) + (x * 3) + 2] = col.g;
    }
  }

  return colours;
}
#include "../include/fluidEngine.h"

#include <cstddef>
#include <cstdio>

#include "../include/core.h"
#include "../include/renderEngine.h"

fluidEngine::fluidEngine(){};
fluidEngine::~fluidEngine(){};
void fluidEngine::AddSandAtPos(int x, int y) { sand[x][y] = true; };
void fluidEngine::Start() {
  for (int x = 0; x < FB_SIZE; x++) {
    for (int y = 0; y < FB_SIZE; y++) {
      sand[x][y] = false;
    }
  }
  printf("Fluid Initialised\n");
};
void fluidEngine::Update() {
  for (int x = 0; x < FB_SIZE; x++) {
    for (int y = 0; y < FB_SIZE; y++) {
      if (y != 0) {
        if (sand[(FB_SIZE - 1) - x][(FB_SIZE - 1) - y] &&
            !sand[(FB_SIZE - 1) - x + 1][(FB_SIZE - 1) - y + 1]) {
          // printf("move!");
          sand[(FB_SIZE - 1) - x][(FB_SIZE - 1) - y] = false;
          sand[(FB_SIZE - 1) - x][(FB_SIZE - 1) - y + 1] = true;
        }
      }
    }
  }
};
float* fluidEngine::SandToColour(float colours[]) {
  int w = FB_SIZE, h = FB_SIZE;
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
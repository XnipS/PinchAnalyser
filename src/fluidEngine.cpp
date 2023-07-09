#include "../include/fluidEngine.h"

#include <cstddef>
#include <cstdio>

#include "../include/core.h"
#include "../include/renderEngine.h"

fluidEngine::fluidEngine(){};
fluidEngine::~fluidEngine(){};
void fluidEngine::AddSandAtPos(int x, int y) { sand[x][y].isSand = true; };
void fluidEngine::Start() {
  for (int x = 0; x < FB_SIZE; x++) {
    for (int y = 0; y < FB_SIZE; y++) {
      sand[x][y].isSand = false;
    }
  }
  printf("Fluid Initialised\n");
};
void fluidEngine::Update() {
  for (int x = FB_SIZE - 1; x >= 0; x--) {
    for (int y = FB_SIZE - 1; y >= 0; y--) {
      if (y != FB_SIZE - 1) {
        if (sand[x][y].isSand && !sand[x][y + 1].isSand) {
          // printf("move!");
          sand[x][y].isSand = false;
          sand[x][y + 1].isSand = true;
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
      if (sand[x][y].isSand) {
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
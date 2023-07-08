#pragma once
#include <vector>

#include "core.h"

class fluidEngine {
 public:
  fluidEngine();
  ~fluidEngine();
  void Start();
  void Update();
  void AddSandAtPos(int x, int y);
  float* SandToColour(float colours[]);

 private:
  bool sand[FB_SIZE][FB_SIZE];  // TODO
};
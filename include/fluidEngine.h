#pragma once
#include <vector>

#include "core.h"

struct Vector2 {
 public:
  Vector2(float xPos, float yPos) {
    x = xPos;
    y = yPos;
  }
  float x, y;
};

class fluidParticle {
 public:
  bool isSand = false;
  Vector2 velocity = *new Vector2(0, 0);
};

class fluidEngine {
 public:
  fluidEngine();
  ~fluidEngine();
  void Start();
  void Update();
  void AddSandAtPos(int x, int y);
  float* SandToColour(float colours[]);

 private:
  fluidParticle sand[FB_SIZE][FB_SIZE];  // TODO
};
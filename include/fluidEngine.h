#pragma once
#include <X11/X.h>

#include <vector>

#include "renderEngine.h"

struct Vector2 {
 public:
  Vector2(double xPos, double yPos) {
    x = xPos;
    y = yPos;
  }
  double x, y;
};

struct Vector2Int {
 public:
  Vector2Int(int xPos, int yPos) {
    x = xPos;
    y = yPos;
  }
  int x, y;
};

class fluidParticle {
 public:
  Vector2 position = *new Vector2(0, 0);
  Vector2 velocity = *new Vector2(0, 0);
  float mass = 1.0;
  fluidParticle(int x, int y) { position = *new Vector2(x, y); }
};

class fluidEngine {
 public:
  fluidEngine();
  ~fluidEngine();
  void Start(renderEngine* ren);
  void Update();
  void AddSandAtPos(int x, int y);
  float* SandToColour(float colours[]);

 private:
  std::vector<fluidParticle> sand;
};
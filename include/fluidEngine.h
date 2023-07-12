#pragma once
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
  void SandToColour(float colours[]);
  int SandCount() { return sand.size(); }
  float cfg_gravity = 0.5;
  float cfg_dampen = 0.5;
  float cfg_size = 1;
  float cfg_heat = 0;
  int cfg_fluid_holes = 20;
  float cfg_fluid_power = 1;

 private:
  std::vector<fluidParticle> sand;
};
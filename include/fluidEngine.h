#pragma once
#include <VectorMath.h>

#include <vector>

#include "renderEngine.h"

class fluidParticle {
 public:
  VM::Vector2 position = *new VM::Vector2(0, 0);
  VM::Vector2 velocity = *new VM::Vector2(0, 0);
  float mass = 1.0;
  float radius = 0.5;
  fluidParticle(int x, int y) { position = *new VM::Vector2(x, y); }
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
  FluidEngineSettings settings;

 private:
  std::vector<fluidParticle> sand;
};
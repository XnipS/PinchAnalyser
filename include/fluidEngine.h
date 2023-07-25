#pragma once
#include <VectorMath.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cmath>
#include <vector>

#include "renderEngine.h"

class fluidParticle {
 public:
  VM::Vector2 position = *new VM::Vector2(0, 0);
  VM::Vector2 velocity = *new VM::Vector2(0, 0);
  VM::Vector2 acceleration = *new VM::Vector2(0, 0);
  float mass = 0.02;      // kg
  float radius = 0.0003;  // m
  fluidParticle(double x, double y, double m, double r) {
    position = *new VM::Vector2(x, y);
    mass = m;
    radius = r;
  }
  double crossSectionalArea() { return M_PI * radius * radius; }  // m**2
  double density() {
    return mass / ((4.0 / 3.0) * M_PI * radius * radius * radius);  // kg/m**3
  }
};

class fluidEngine {
 public:
  fluidEngine();
  ~fluidEngine();
  void Start(renderEngine* ren);
  void Update();
  void AddSandAtRnd();
  void AddSandAtPos(double x, double y);
  void SandToColour(float colours[]);
  void LinkSandToMain(std::vector<CircleSettings>* newPositions);
  int SandCount() { return sand.size(); }
  FluidEngineSettings settings;

 private:
  void CollisionUpdate();
  void Reflect(double* input);
  std::vector<fluidParticle> sand;
};
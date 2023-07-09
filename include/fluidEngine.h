#pragma once
#include <X11/X.h>

#include <vector>

struct Vector2 {
 public:
  Vector2(float xPos, float yPos) {
    x = xPos;
    y = yPos;
  }
  float x, y;
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
  bool isSand = false;
  Vector2 position = *new Vector2(0, 0);
  Vector2 velocity = *new Vector2(0, 0);
  fluidParticle(int x, int y) { position = *new Vector2(x, y); }
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
  std::vector<fluidParticle> sand;
};
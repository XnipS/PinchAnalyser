#pragma once
#include <vector>

class fluidEngine {
 public:
  fluidEngine();
  ~fluidEngine();
  void Start();
  void Update();
  void AddSandAtPos(int x, int y);

 private:
  bool sand[500][500];  // TODO
};
#pragma once
#include <VectorMath.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct inputStream {
  int temp_initial = 100;
  int temp_target = 50;
  float cx = 1.5;
};

class exchangerEngine {
 public:
  exchangerEngine();
  ~exchangerEngine();
  void Update();

 private:
};
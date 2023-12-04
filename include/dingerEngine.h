#pragma once
#include <VectorMath.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cmath>
#include <vector>

#include "renderEngine.h"

class dingerEngine {
 public:
  dingerEngine();
  ~dingerEngine();
  void Start(renderEngine* ren);
  void Update();
  FluidEngineSettings settings;

 private:
};
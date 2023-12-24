#include "../include/exchangerEngine.h"

#include <cmath>
#include <cstdio>
#include <sstream>

#include "../include/core.h"
#include "../include/renderEngine.h"
#include "VectorMath.h"

exchangerEngine::exchangerEngine(){};
exchangerEngine::~exchangerEngine(){};

renderEngine* renderer;

// Initialise fluid engine
void exchangerEngine::Start(renderEngine* ren) {
  printf("Fluid Engine Initialised\n");
  renderer = ren;
};

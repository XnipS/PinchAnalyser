#include "../include/dingerEngine.h"

#include <cmath>
#include <cstdio>
#include <sstream>

#include "../include/core.h"
#include "../include/renderEngine.h"
#include "VectorMath.h"

dingerEngine::dingerEngine(){};
dingerEngine::~dingerEngine(){};

renderEngine* renderer;

// Initialise fluid engine
void dingerEngine::Start(renderEngine* ren) {
  printf("Fluid Engine Initialised\n");
  renderer = ren;
};

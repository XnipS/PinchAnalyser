#include "../include/fluidEngine.h"

#include <math.h>

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <new>
#include <ostream>

#include "../include/core.h"
#include "../include/renderEngine.h"

fluidEngine::fluidEngine(){};
fluidEngine::~fluidEngine(){};
void fluidEngine::AddSandAtPos(int x, int y) {
  fluidParticle particle = *new fluidParticle(x, y);
  particle.velocity.y = -1;
  particle.velocity.x = 2;
  sand.push_back(particle);
};

Vector2Int VectorRoundToInt(Vector2* input) {
  Vector2Int output = *new Vector2Int(0, 0);
  output.x = std::round(input->x);
  output.y = std::round(input->y);
  return output;
}
float VectorDistance(Vector2* u, Vector2* v) {
  float output = std::sqrt(pow((u->x - v->x), 2) + pow((u->y - v->y), 2));
  return output;
}
Vector2 VectorDotProduct(Vector2* i, Vector2* j) {
  Vector2 output = *new Vector2(0, 0);
  output.x = i->x + j->x;
  output.y = i->y + j->y;
  return output;
}
Vector2 VectorScalar(Vector2* i, float j) {
  Vector2 output = *new Vector2(0, 0);
  output.x = i->x * j;
  output.y = i->y * j;
  return output;
}

void fluidEngine::Start() {
  printf("Fluid Engine Initialised\n");

  // Test rounding
  // Vector2 newVec = *new Vector2(0.5f, 1.2f);
  // Vector2Int vec = ClampVelocity(&newVec);
  // printf("x = %i, y = %i", vec.x, vec.y);
};

void fluidEngine::Update() {
  // printf("Fluid Count: %zu\n", sand.size());
  for (int i = 0; i < sand.size(); i++) {
    Vector2 nextPos = VectorDotProduct(&sand[i].position, &sand[i].velocity);
    // nextPos = VectorScalar(&nextPos, FB_DELTATIME);
    Vector2Int round = VectorRoundToInt(&nextPos);
    // printf("%d", FB_DELTATIME);
    // printf("Pos: %f, Velocity: %f, Next: %f\n", sand[i].position.y,
    //        sand[i].velocity.y, nextPos.y);
    if (round.x < 0 || round.x > (FB_SIZE - 1)) {
      sand[i].velocity.x = -sand[i].velocity.x;
    } else if (round.y < 0 || round.y > (FB_SIZE - 1)) {
      sand[i].velocity.y = -sand[i].velocity.y;
    } else {
      sand[i].position = nextPos;
    }
    // for (int j = 0; j < sand.size(); j++) {
    //   if (i != j) {
    //     float distance = VectorDistance(&nextPos, &sand[j].position);
    //     if (distance <= 1) {
    //       // Collision
    //       // sand[i].position
    //     } else {
    //       // Movement
    //       sand[i].position = nextPos;
    //     }
    //   }
    // }
  }
};
float* fluidEngine::SandToColour(float colours[]) {
  int w = FB_SIZE, h = FB_SIZE;
  Colour3 white;
  white.r = 1;
  white.g = 1;
  white.b = 1;
  Colour3 grey;
  grey.r = .2;
  grey.g = .2;
  grey.b = .2;
  for (int x = 0; x < (w); x++) {
    for (int y = 0; y < (h); y++) {
      colours[(y * w * 3) + (x * 3)] = grey.r;
      colours[(y * w * 3) + (x * 3) + 1] = grey.b;
      colours[(y * w * 3) + (x * 3) + 2] = grey.g;
    }
  }

  for (int i = 0; i < sand.size(); i++) {
    Vector2Int rounded = VectorRoundToInt(&sand[i].position);
    // std::cout << rounded.x << std::endl;
    colours[(rounded.y * w * 3) + (rounded.x * 3)] = white.r;
    colours[(rounded.y * w * 3) + (rounded.x * 3) + 1] = white.b;
    colours[(rounded.y * w * 3) + (rounded.x * 3) + 2] = white.g;
  }

  return colours;
}
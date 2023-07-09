#include "../include/fluidEngine.h"

#include <X11/X.h>
#include <math.h>

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <new>
#include <ostream>
#include <sstream>
#include <string>

#include "../include/core.h"
#include "../include/renderEngine.h"

fluidEngine::fluidEngine(){};
fluidEngine::~fluidEngine(){};

renderEngine* renderer;

void fluidEngine::AddSandAtPos(int x, int y) {
  fluidParticle particle = *new fluidParticle(x, y);
  particle.velocity.y = -0.1;
  particle.velocity.x = 0.2;
  sand.push_back(particle);
};

std::vector<fluidParticle> Duplicate(std::vector<fluidParticle>& input) {
  return input;
};

Vector2Int VectorRoundToInt(Vector2* input) {
  Vector2Int output = *new Vector2Int(0, 0);
  output.x = std::round(input->x);
  output.y = std::round(input->y);
  return output;
}
double VectorMagnitude(Vector2* input) {
  double magnitude = std::sqrt(input->x * input->x + input->y * input->y);
  return magnitude;
}
Vector2 VectorNormalise(Vector2* input) {
  Vector2 output = *new Vector2(input->x, input->y);
  double magnitude = VectorMagnitude(input);
  if (magnitude != 0.0) {
    output.x /= magnitude;
    output.y /= magnitude;
  }

  return output;
}
float VectorDistance(Vector2* u, Vector2* v) {
  float output = std::sqrt(pow((u->x - v->x), 2) + pow((u->y - v->y), 2));
  return output;
}
Vector2 VectorSum(Vector2* i, Vector2* j) {
  Vector2 output = *new Vector2(0, 0);
  output.x = i->x + j->x;
  output.y = i->y + j->y;
  return output;
}
Vector2 VectorSubtraction(Vector2* i, Vector2* j) {
  Vector2 output = *new Vector2(0, 0);
  output.x = i->x - j->x;
  output.y = i->y - j->y;
  return output;
}
float VectorDot(Vector2* i, Vector2* j) {
  float output = 0;
  output += i->x * j->x;
  output += i->y * j->y;
  return output;
}
Vector2 VectorScalar(Vector2* i, float j) {
  Vector2 output = *new Vector2(0, 0);
  output.x = i->x * j;
  output.y = i->y * j;
  return output;
}

void fluidEngine::Start(renderEngine* ren) {
  printf("Fluid Engine Initialised\n");
  renderer = ren;
  // Test rounding
  // Vector2 newVec = *new Vector2(0.5f, 1.2f);
  // Vector2Int vec = ClampVelocity(&newVec);
  // printf("x = %i, y = %i", vec.x, vec.y);
};

void fluidEngine::Update() {
  // printf("Fluid Count: %zu\n", sand.size());
  float energy = 0;
  for (int t = 0; t < sand.size(); t++) {
    // energy += VectorMagnitude(&sand[t].velocity);
    float magnitude = VectorMagnitude(&sand[t].velocity);
    energy += 0.5 * magnitude * magnitude;
  }
  if (renderer->currentDebugInfo.size() >= 2) {
    std::ostringstream ss;
    ss << "Total Energy: ";
    ss << std::to_string(energy);
    ss << " J";
    renderer->currentDebugInfo[1] = ss.str();
  } else {
    std::ostringstream ss;
    ss << "Starting Energy: ";
    ss << std::to_string(energy);
    ss << " J";
    renderer->currentDebugInfo.push_back(ss.str());
    renderer->currentDebugInfo.push_back("INCOMING!");
  }
  // printf("Energy: %f\n", energy);

  std::vector<fluidParticle> oldSand = Duplicate(sand);

  for (int i = 0; i < oldSand.size(); i++) {
    Vector2 nextPos = VectorSum(&sand[i].position, &sand[i].velocity);
    Vector2Int round = VectorRoundToInt(&nextPos);
    // printf("%d", FB_DELTATIME);
    // printf("Pos: %f, Velocity: %f, Next: %f\n", sand[i].position.y,
    //        sand[i].velocity.y, nextPos.y);
    if (round.x < 0 || round.x > (FB_SIZE - 1)) {
      sand[i].velocity.x = -sand[i].velocity.x;
    } else if (round.y < 0 || round.y > (FB_SIZE - 1)) {
      sand[i].velocity.y = -sand[i].velocity.y;
    } else {
      // sand[i].position = nextPos;
      //}

      for (int j = 0; j < oldSand.size(); j++) {
        if (i != j) {
          double dx = sand[j].position.x - sand[i].position.x;
          double dy = sand[j].position.y - sand[i].position.y;
          double distance = std::sqrt(dx * dx + dy * dy);
          /*           if (distance <= 1) {
                      sand[i].velocity.x = -sand[i].velocity.x;
                      sand[i].velocity.y = -sand[i].velocity.y;
                      oldSand[j].velocity.x = -oldSand[j].velocity.x;
                      oldSand[j].velocity.y = -oldSand[j].velocity.y;
                      while (distance <= 1) {
                        nextPos = VectorSum(&sand[i].position,
             &sand[i].velocity); sand[i].position = nextPos;

                        dx = sand[j].position.x - sand[i].position.x;
                        dy = sand[j].position.y - sand[i].position.y;
                        distance = std::sqrt(dx * dx + dy * dy);
                      }
                    } */
          if (distance <= 1) {
            double nx = dx / distance;
            double ny = dy / distance;

            // Calculate the relative velocity components along the collision
            // vector
            double v1n = sand[i].velocity.x * nx + sand[i].velocity.y * ny;
            double v2n = sand[j].velocity.x * nx + sand[j].velocity.y * ny;

            // Calculate the new normal velocity components after the collision
            double v1n_after =
                (v1n * (sand[i].mass - sand[j].mass) + 2 * sand[j].mass * v2n) /
                (sand[i].mass + sand[j].mass);
            double v2n_after =
                (v2n * (sand[j].mass - sand[i].mass) + 2 * sand[i].mass * v1n) /
                (sand[i].mass + sand[j].mass);

            // Calculate the change in normal velocity
            double dv1n = v1n_after - v1n;
            double dv2n = v2n_after - v2n;

            // Update the velocities of the circles after the collision
            sand[i].velocity.x += dv1n * nx;
            sand[i].velocity.y += dv1n * ny;
            sand[j].velocity.x += dv2n * nx;
            sand[j].velocity.y += dv2n * ny;

            while (distance <= 1) {
              nextPos = VectorSum(&sand[i].position, &sand[i].velocity);
              sand[i].position = nextPos;

              dx = sand[j].position.x - sand[i].position.x;
              dy = sand[j].position.y - sand[i].position.y;
              distance = std::sqrt(dx * dx + dy * dy);
            }

          } else {
            // Movement
            sand[i].position = nextPos;
          }
        }
      }
    }
  }
}

;
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
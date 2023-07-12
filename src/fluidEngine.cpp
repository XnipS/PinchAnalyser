#include "../include/fluidEngine.h"

#include <cmath>
#include <sstream>

#include "../include/core.h"
#include "../include/renderEngine.h"

fluidEngine::fluidEngine(){};
fluidEngine::~fluidEngine(){};

renderEngine* renderer;

// Spawn new particle
void fluidEngine::AddSandAtPos(int x, int y) {
  fluidParticle particle = *new fluidParticle(x, y);
  particle.velocity.y = 0.1;  //-0.1;
  particle.velocity.x = 0.1;  // 0.2;
  sand.push_back(particle);
};
// Snap vector to grid
static void VectorRoundToInt(Vector2* input, Vector2Int* output) {
  output->x = std::floor(input->x);
  output->y = std::floor(input->y);
}
// Get magnitude of vector
static double VectorMagnitude(Vector2* input) {
  double magnitude = std::sqrt(input->x * input->x + input->y * input->y);
  return magnitude;
}
// Get distance between two vectors
static float VectorDistance(Vector2* u, Vector2* v) {
  float output = std::sqrt(pow((u->x - v->x), 2) + pow((u->y - v->y), 2));
  return output;
}
// Sum two vectors together
static Vector2 VectorSum(Vector2* i, Vector2* j) {
  i->x += j->x;
  i->y += j->y;
  return *i;
}
// Sum vector by scalar
static Vector2 VectorSumScalar(Vector2* i, float j) {
  if (i->x > 0) {
    i->x += j;
  } else {
    i->x -= j;
  }

  if (i->y > 0) {
    i->y += j;
  } else {
    i->y -= j;
  }
  return *i;
}

void fluidEngine::Start(renderEngine* ren) {
  printf("Fluid Engine Initialised\n");
  renderer = ren;
};

void fluidEngine::Update() {
  // TOTAL KINETIC ENERGY
  float energy = 0;
  for (int t = 0; t < sand.size(); t++) {
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

  for (int i = 0; i < sand.size(); i++) {
    // Sum gravity
    sand[i].velocity.y += cfg_gravity;
    // Sum heat energy
    VectorSumScalar(&sand[i].velocity, cfg_heat);
    // Calculate next position
    Vector2 nextPos = VectorSum(&sand[i].position, &sand[i].velocity);
    Vector2Int round(0, 0);
    VectorRoundToInt(&nextPos, &round);
    // Keep/collide in/with container
    if (round.x < 0 && sand[i].velocity.x < 0) {
      sand[i].position.x = 0;
      sand[i].velocity.x = -sand[i].velocity.x * (1.0 - cfg_dampen);
    } else if (round.x > (FB_SIZE - 1) && sand[i].velocity.x > 0) {
      sand[i].position.x = FB_SIZE - 1;
      sand[i].velocity.x = -sand[i].velocity.x * (1.0 - cfg_dampen);
    } else if (round.y < 0 && sand[i].velocity.y < 0) {
      sand[i].position.y = 0;
      sand[i].velocity.y = -sand[i].velocity.y * (1.0 - cfg_dampen);
    } else if (round.y > (FB_SIZE - 1) && sand[i].velocity.y > 0) {
      sand[i].position.y = FB_SIZE - 1;
      sand[i].velocity.y = -sand[i].velocity.y * (1.0 - cfg_dampen);
    } else {
      // Collision check other particles
      for (int j = 0; j < sand.size(); j++) {
        if (i != j) {
          double dx = sand[j].position.x - sand[i].position.x;
          double dy = sand[j].position.y - sand[i].position.y;

          if (dx <= cfg_size * 2 && dy <= cfg_size * 2) {
            double distance = std::sqrt(dx * dx + dy * dy);

            if (distance <= cfg_size) {
              if (distance == 0) {
                printf("Zero distance\n");
                continue;
              }
              double nx = dx / distance;
              double ny = dy / distance;

              // Calculate the relative velocity components along the collision
              // vector
              double v1n = sand[i].velocity.x * nx + sand[i].velocity.y * ny;
              double v2n = sand[j].velocity.x * nx + sand[j].velocity.y * ny;

              // Calculate the new normal velocity components after the
              // collision
              double v1n_after = (v1n * (sand[i].mass - sand[j].mass) +
                                  2 * sand[j].mass * v2n) /
                                 (sand[i].mass + sand[j].mass);
              double v2n_after = (v2n * (sand[j].mass - sand[i].mass) +
                                  2 * sand[i].mass * v1n) /
                                 (sand[i].mass + sand[j].mass);

              // Dampen
              v1n_after *= (1.0 - cfg_dampen);
              v2n_after *= (1.0 - cfg_dampen);

              // Calculate the change in normal velocity
              double dv1n = v1n_after - v1n;
              double dv2n = v2n_after - v2n;

              // Update the velocities of the circles after the collision
              sand[i].velocity.x += dv1n * nx;
              sand[i].velocity.y += dv1n * ny;
              sand[j].velocity.x += dv2n * nx;
              sand[j].velocity.y += dv2n * ny;

              // Find suitable location
              while (distance <= cfg_size) {
                nextPos = VectorSum(&sand[i].position, &sand[i].velocity);
                sand[i].position = nextPos;

                dx = sand[j].position.x - sand[i].position.x;
                dy = sand[j].position.y - sand[i].position.y;
                distance = std::sqrt(dx * dx + dy * dy);
                if (distance == 0) {
                  printf("Zero distance\n");
                  break;
                }
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
}

void fluidEngine::SandToColour(float colours[]) {
  const Colour3 white(1, 1, 1);
  const Colour3 red(1, 0, 0);
  const Colour3 grey(.2, .2, .2);

  for (int x = 0; x < (FB_SIZE); x++) {
    for (int y = 0; y < (FB_SIZE); y++) {
      colours[(y * FB_SIZE * 3) + (x * 3)] = grey.r;
      colours[(y * FB_SIZE * 3) + (x * 3) + 1] = grey.b;
      colours[(y * FB_SIZE * 3) + (x * 3) + 2] = grey.g;
    }
  }

  for (int i = 0; i < sand.size(); i++) {
    Vector2Int rounded(0, 0);
    VectorRoundToInt(&sand[i].position, &rounded);

    if (rounded.x < 0 || rounded.x > (FB_SIZE - 1) || rounded.y < 0 ||
        rounded.y > (FB_SIZE - 1)) {
      // OUT OF BOUNDS
      // printf("Out of bounds @ (%i,%i)!", rounded.x, rounded.y);
    } else {
      // std::cout << rounded.x << std::endl;
      if (i == 0) {
        // Marker to follow
        colours[(rounded.y * FB_SIZE * 3) + (rounded.x * 3)] = red.r;
        colours[(rounded.y * FB_SIZE * 3) + (rounded.x * 3) + 1] = red.b;
        colours[(rounded.y * FB_SIZE * 3) + (rounded.x * 3) + 2] = red.g;
      } else {
        colours[(rounded.y * FB_SIZE * 3) + (rounded.x * 3)] = white.r;
        colours[(rounded.y * FB_SIZE * 3) + (rounded.x * 3) + 1] = white.b;
        colours[(rounded.y * FB_SIZE * 3) + (rounded.x * 3) + 2] = white.g;
      }

      // Visualise fluid holes TODO
    }
  }
}
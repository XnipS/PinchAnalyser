#include "../include/fluidEngine.h"

#include <cmath>
#include <cstdio>
#include <sstream>

#include "../include/core.h"
#include "../include/renderEngine.h"

fluidEngine::fluidEngine(){};
fluidEngine::~fluidEngine(){};

renderEngine* renderer;
VM::Vector2 startingVelocity(0.0001, 0.0001);

double RandomRange(double fMin, double fMax) {
  double f = (double)rand() / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

// Spawn new particle
void fluidEngine::AddSandAtPos(double x, double y) {
  fluidParticle particle =
      *new fluidParticle(x, y, settings.mass, settings.radius);
  particle.velocity = startingVelocity;
  sand.push_back(particle);
};

// Spawn new particle
void fluidEngine::AddSandAtRnd() {
  AddSandAtPos(RandomRange(0, FB_CONTAINER_SIZE),
               RandomRange(0, FB_CONTAINER_SIZE));
};

// Initialise fluid engine
void fluidEngine::Start(renderEngine* ren) {
  printf("Fluid Engine Initialised\n");
  renderer = ren;
};

// Keep/collide in/with container
void fluidEngine::Reflect(double* input) { *input *= -(1.0 - settings.dampen); }

// Fluid engine tick
void fluidEngine::Update() {
  // Add sand on tick
  if (renderer->AddSand() != 0) {
    for (int i = 0; i < renderer->AddSand(); i++) {
      AddSandAtRnd();
    }
  }
  // Remove sand on clear button
  if (renderer->ClearSand()) {
    sand.clear();
  }
  // Calculate approximate total kinetic energy
  float energy = 0;
  for (int t = 0; t < sand.size(); t++) {
    double magnitude;
    VectorMagnitude(&sand[t].velocity, &magnitude);
    energy += 0.5 * magnitude * magnitude;
  }
  if (renderer->currentDebugInfo.size() >= 2) {
    std::ostringstream ss;
    ss << "Current Energy: ";
    ss << std::to_string(energy);
    ss << " J";
    renderer->currentDebugInfo[1] = ss.str();

    double starting;
    VectorMagnitude(&startingVelocity, &starting);

    starting = (0.5 * starting * starting) * SandCount();

    std::ostringstream sss;
    sss << "Starting Energy: ";
    sss << std::to_string(starting);
    sss << " J";
    renderer->currentDebugInfo[0] = sss.str();

  } else {
    renderer->currentDebugInfo.push_back("INCOMING!");
    renderer->currentDebugInfo.push_back("INCOMING!");
  }

  // Physics tick
  for (int i = 0; i < sand.size(); i++) {
    // Gravity
    double gravityAccel = settings.gravity;  // a = m/s^2 = const
    gravityAccel *= FB_DELTATIME;            // a = a * t
    sand[i].velocity.y += gravityAccel;      // v = v0 + at

    // Sum heat energy OLD
    // VectorSumScalar(&sand[i].velocity, settings.heat);

    // Apply drag
    /* if (round.x % (FB_SIZE / settings.fluid_holes) == 0) {
      double dragForce = 0.5 * sand[i].density() *
                         sand[i].crossSectionalArea() *
                         FB_MOLECULE_DRAGCOEFFICIENT;  // F = N
    (Scalar)

      double relativeX = newVelocity.x;
      double relativeY = newVelocity.y - settings.fluid_power;

      relativeX = dragForce * relativeX * relativeX;
      relativeY = dragForce * relativeX * relativeY;

      VM::Vector2 drag(relativeX, relativeY);  // F = N (Vector)

      VM::VectorScalarDivide(&drag, &drag, sand[i].mass);  // a =
    F / m

      VM::VectorScalarMultiply(&drag, &drag,  // v = a * t
                               FB_DELTATIME);

      VM::VectorSum(&newVelocity, &newVelocity, &drag);
    } */

    VM::Vector2 nextPos(0, 0);
    // Collision check other particles
    for (int j = 0; j < sand.size(); j++) {
      if (i != j) {
        double distance;
        VectorDistance(&sand[i].position, &sand[j].position, &distance);

        if (distance < sand[i].radius + sand[j].radius) {
          if (distance == 0) {
            printf("Zero distance\n");
            distance = sand[i].radius + sand[j].radius;
          }

          double dx = sand[j].position.x - sand[i].position.x;
          double dy = sand[j].position.y - sand[i].position.y;

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

          // Dampen
          v1n_after *= (1.0 - settings.dampen);
          v2n_after *= (1.0 - settings.dampen);

          // Calculate the change in normal velocity
          double dv1n = v1n_after - v1n;
          double dv2n = v2n_after - v2n;

          // Update the velocities of the circles after the collision
          sand[i].velocity.x += dv1n * nx;
          sand[i].velocity.y += dv1n * ny;
          sand[j].velocity.x += dv2n * nx;
          sand[j].velocity.y += dv2n * ny;

          // Find suitable location
          if (distance < sand[i].radius + sand[j].radius) {
            // Get dir away from collision
            VectorSubtract(&nextPos, &sand[j].position, &sand[i].position);
            // Normalise
            VectorNormalise(&nextPos);
            // Multiply by radius + extra
            VectorScalarMultiply(
                &nextPos, &nextPos,
                (sand[i].radius + sand[j].radius) * -FB_MOLECULE_COL_BOUNDARY);
            // Transform to position
            VectorSum(&nextPos, &sand[i].position, &nextPos);
            // Movement
            sand[i].position = nextPos;
          }
        }
      }
    }

    // Stop invalid velocities
    if (sand[i].velocity.x > FB_CONTAINER_SIZE ||
        sand[i].velocity.y > FB_CONTAINER_SIZE) {
      printf("Weird velocity clamped!");
      sand[i].velocity.x = 0;
      sand[i].velocity.y = 0;
    }

    // Calculate next position
    VectorSum(&nextPos, &sand[i].position, &sand[i].velocity);
    sand[i].position = nextPos;

    // Keep/collide in/with container
    // Horizontal
    if (sand[i].position.x < 0 + sand[i].radius && sand[i].velocity.x < 0) {
      sand[i].position.x = 0;
      Reflect(&sand[i].velocity.x);
    } else if (sand[i].position.x > (FB_CONTAINER_SIZE - sand[i].radius) &&
               sand[i].velocity.x > 0) {
      sand[i].position.x = FB_CONTAINER_SIZE;
      Reflect(&sand[i].velocity.x);
    }
    // Vertical
    if (sand[i].position.y < 0 + sand[i].radius && sand[i].velocity.y < 0) {
      sand[i].position.y = 0;
      Reflect(&sand[i].velocity.y);
    } else if (sand[i].position.y > (FB_CONTAINER_SIZE - sand[i].radius) &&
               sand[i].velocity.y > 0) {
      sand[i].position.y = FB_CONTAINER_SIZE;
      Reflect(&sand[i].velocity.y);
    }
  }
}

// Encode sand data to colour data
void fluidEngine::SandToColour(float colours[]) {
  const Colour3 white(1, 1, 1);
  const Colour3 blue(0, 1, 0);
  const Colour3 red(1, 0, 0);
  const Colour3 grey(.2, .2, .2);

  // Clear old colours
  for (int x = 0; x < (FB_CONTAINER_OUTPUT); x++) {
    for (int y = 0; y < (FB_CONTAINER_OUTPUT); y++) {
      // Visualise fluid holes
      //   if ((x % (FB_CONTAINER_OUTPUT / settings.fluid_holes) == 0 &&
      //        y == FB_CONTAINER_OUTPUT - 1)) {
      //     colours[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3)] = blue.r;
      //     colours[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3) + 1] = blue.b;
      //     colours[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3) + 2] = blue.g;
      //   } else {
      colours[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3)] = grey.r;
      colours[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3) + 1] = grey.b;
      colours[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3) + 2] = grey.g;
      //}
    }
  }

  // Render sand
  for (int i = 0; i < sand.size(); i++) {
    // Rounding
    VM::Vector2Int rounded(0, 0);
    static double scale = (FB_CONTAINER_OUTPUT - 1) / FB_CONTAINER_SIZE;
    rounded.x = (std::ceil(sand[i].position.x * scale));
    rounded.y = (std::ceil(sand[i].position.y * scale));

    if (rounded.x < 0 || rounded.x > (FB_CONTAINER_OUTPUT - 1) ||
        rounded.y < 0 || rounded.y > (FB_CONTAINER_OUTPUT - 1)) {
      // OUT OF BOUNDS
      // printf("Out of bounds @ (%f,%f)!", sand[i].position.x,
      //        sand[i].position.y);
    } else {
      // std::cout << rounded.x << std::endl;
      if (i == 0) {
        // Marker to follow
        colours[(rounded.y * FB_CONTAINER_OUTPUT * 3) + (rounded.x * 3)] =
            red.r;
        colours[(rounded.y * FB_CONTAINER_OUTPUT * 3) + (rounded.x * 3) + 1] =
            red.b;
        colours[(rounded.y * FB_CONTAINER_OUTPUT * 3) + (rounded.x * 3) + 2] =
            red.g;
      } else {
        colours[(rounded.y * FB_CONTAINER_OUTPUT * 3) + (rounded.x * 3)] =
            white.r;
        colours[(rounded.y * FB_CONTAINER_OUTPUT * 3) + (rounded.x * 3) + 1] =
            white.b;
        colours[(rounded.y * FB_CONTAINER_OUTPUT * 3) + (rounded.x * 3) + 2] =
            white.g;
      }
    }
  }
}
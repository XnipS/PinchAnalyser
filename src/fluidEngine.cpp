#include "../include/fluidEngine.h"

#include <cmath>
#include <cstdio>
#include <sstream>

#include "../include/core.h"
#include "../include/renderEngine.h"
#include "VectorMath.h"

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

// Do collision check on all particles
void fluidEngine::CollisionUpdate() {
  VM::Vector2 collisionAxis(0, 0);
  for (int i = 0; i < sand.size(); i++) {
    for (int j = 0; j < sand.size(); j++) {
      if (i != j) {
        double dist;
        VectorDistance(&sand[i].position, &sand[j].position, &dist);
        const double min_dist = sand[i].radius + sand[j].radius;
        if (dist < min_dist) {
          if (dist == 0) {
            printf("Zero distance\n");
            // distance = sand[i].radius + sand[j].radius;
            continue;
          }

          VM::VectorSubtract(&collisionAxis, &sand[j].position,
                             &sand[i].position);

          VM::VectorNormalise(&collisionAxis);
          const double delta = dist - (sand[i].radius + sand[j].radius);
          VM::VectorScalarMultiply(&collisionAxis, &collisionAxis,
                                   0.5f * delta);

          VM::VectorSum(&sand[i].position, &sand[i].position, &collisionAxis);
          VM::VectorSubtract(&sand[j].position, &sand[j].position,
                             &collisionAxis);
        }
      }
    }
  }
}

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

  // Physics steps
  for (int i = 0; i < FB_MOLECULE_PHYSIC_STEPS; i++) {
    CollisionUpdate();
  }

  // Physics tick
  for (int i = 0; i < sand.size(); i++) {
    // NEXT POSITION
    VM::Vector2 new_pos(sand[i].position);
    VM::Vector2 old_acc(sand[i].acceleration);
    VM::Vector2 old_vel(sand[i].velocity);
    VectorScalarMultiply(&old_acc, &old_acc,
                         (0.5 * FB_DELTATIME * FB_DELTATIME));
    VectorScalarMultiply(&old_vel, &old_vel, (FB_DELTATIME));
    VectorSum(&new_pos, &new_pos, &old_acc);
    VectorSum(&new_pos, &new_pos, &old_vel);

    // NEXT ACCELERATION
    VM::Vector2 new_acc(0, settings.gravity);
    double magnitude;
    VM::VectorMagnitude(&sand[i].velocity, &magnitude);
    VM::Vector2 drag_force(
        0, 0.5 * sand[i].density() * sand[i].crossSectionalArea() *
               settings.dragCoefficient * (magnitude * magnitude));
    VM::VectorScalarDivide(&drag_force, &drag_force, sand[i].mass);
    VM::VectorSubtract(&new_acc, &new_acc, &drag_force);

    // NEXT VELOCITY
    VM::Vector2 new_vel(0, 0);
    VectorSum(&new_vel, &sand[i].acceleration, &new_acc);
    VM::VectorScalarMultiply(&new_vel, &new_vel, FB_DELTATIME * 0.5);
    VectorSum(&new_vel, &new_vel, &sand[i].velocity);

    // UPDATE
    sand[i].position = new_pos;
    sand[i].velocity = new_vel;
    sand[i].acceleration = new_acc;

    // Stop invalid velocities
    if (sand[i].velocity.x > FB_CONTAINER_SIZE ||
        sand[i].velocity.y > FB_CONTAINER_SIZE) {
      printf("Weird velocity clamped!");
      sand[i].velocity.x = 0;
      sand[i].velocity.y = 0;
    }

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

// Encode sand data to colour data
void fluidEngine::LinkSandToMain(
    std::vector<CircleSettings>* updatedParticles) {
  // Render sand
  if (renderer->ClearSand()) {
    updatedParticles->clear();
  }

  static double scale =
      ((FB_CONTAINER_OUTPUT - 1) / FB_CONTAINER_SIZE) * FB_IMAGE_SCALE_V2;

  for (int i = 0; i < sand.size(); i++) {
    // Rounding
    VM::Vector2 temp = sand[i].position;
    VM::VectorScalarMultiply(&temp, &temp, scale);
    CircleSettings circle(temp, sand[i].radius * scale);
    if (updatedParticles->size() <= i) {
      updatedParticles->push_back(circle);
    } else {
      (*updatedParticles)[i].position = temp;
      (*updatedParticles)[i].radius = sand[i].radius * scale;
    }
  }
}
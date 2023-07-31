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
void fluidEngine::CollisionUpdate(fluidParticle* particle) {
  VM::Vector2 collisionAxis(0, 0);

  for (int j = 0; j < sand.size(); j++) {
    // if (*particle != sand[j]) {
    double dist;
    VectorDistance(&particle->position, &sand[j].position, &dist);
    const double min_dist = particle->radius + sand[j].radius;
    if (dist < min_dist) {
      if (dist == 0) {
        // printf("Zero distance\n");
        //  distance = sand[i].radius + sand[j].radius;
        continue;
      }

      VM::VectorSubtract(&collisionAxis, &sand[j].position,
                         &particle->position);

      VM::VectorNormalise(&collisionAxis);
      const double delta = (particle->radius + sand[j].radius) - dist;
      VM::VectorScalarMultiply(&collisionAxis, &collisionAxis, 0.5f * delta);

      VM::VectorSum(&sand[j].position, &sand[j].position, &collisionAxis);
      VM::VectorSubtract(&particle->position, &particle->position,
                         &collisionAxis);
    }
  }
  //}
}

void fluidEngine::GravityUpdate(fluidParticle* particle) {
  double magnitude;
  VM::Vector2 new_acc(0, 0);

  // Gravity to bottom or centre
  if (settings.useNormalGravity) {
    new_acc.y = settings.gravity;
  } else {
    VM::Vector2 middle(FB_CONTAINER_SIZE / 2, FB_CONTAINER_SIZE / 2);
    VM::VectorSubtract(&new_acc, &middle, &particle->position);
    VM::VectorNormalise(&new_acc);
    VM::VectorScalarMultiply(&new_acc, &new_acc, settings.gravity);
  }

  // Fluid
  static double scale = (FB_CONTAINER_OUTPUT - 1) / FB_CONTAINER_SIZE;
  if (((int)std::round(particle->position.x * scale) %
           (FB_CONTAINER_OUTPUT / settings.fluid_holes) ==
       0)) {
    new_acc.y -=
        ((settings.fluid_power * settings.fluidDensity * particle->position.y) /
         particle->mass);  // F = (rho * h * g) = ma
  }

  // Particle Drag
  VM::Vector2 velocity(0, 0);
  VM::VectorSubtract(&velocity, &particle->position, &particle->position_old);

  VM::VectorMagnitude(&velocity, &magnitude);
  VM::Vector2 drag_force(
      0, 0.5 * particle->density() * particle->crossSectionalArea() *
             settings.dragCoefficient * (magnitude * magnitude));
  VM::VectorScalarDivide(&drag_force, &drag_force, particle->mass);
  VM::VectorSubtract(&particle->acceleration, &new_acc, &drag_force);
};

void fluidEngine::ContainerUpdate(fluidParticle* particle) {
  // Keep/collide in/with container
  // Horizontal
  if (particle->position.x < 0 + particle->radius) {
    particle->position.x = particle->radius;
  } else if (particle->position.x > (FB_CONTAINER_SIZE - particle->radius)) {
    particle->position.x = (FB_CONTAINER_SIZE - particle->radius);
  }
  // Vertical
  if (particle->position.y < 0 + particle->radius) {
    particle->position.y = particle->radius;
  } else if (particle->position.y > (FB_CONTAINER_SIZE - particle->radius)) {
    particle->position.y = (FB_CONTAINER_SIZE - particle->radius);
  }
};
void fluidEngine::PositionUpdate(fluidParticle* particle) {
  VM::Vector2 velocity(0, 0);
  VM::VectorSubtract(&velocity, &particle->position,
                     &particle->position_old);  // delta_x
  particle->position_old = particle->position;  // iterate x

  VM::VectorScalarMultiply(&particle->acceleration, &particle->acceleration,
                           FB_DELTATIME * FB_DELTATIME);  // a * t

  VM::VectorSum(&velocity, &velocity,
                &particle->acceleration);  // v = at + v0

  VM::VectorSum(&particle->position, &particle->position,
                &velocity);  // x = x0 + v

  // VM::VectorScalarMultiply(&particle->acceleration,
  // &particle->acceleration,
  //                          FB_DELTATIME * FB_DELTATIME);

  // VM::VectorSum(&velocity, &velocity, &particle->acceleration);

  // VM::VectorSum(&particle->position, &particle->position, &velocity);

  // particle->acceleration = VM::Vector2(0, 0);
};

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
    VectorMagnitude(&sand[t].position_old, &magnitude);
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
    GravityUpdate(&sand[i]);
    ContainerUpdate(&sand[i]);
    // Physics steps
    for (int x = 0; x < settings.collisionCalcCount; x++) {
      CollisionUpdate(&sand[i]);
    }
    PositionUpdate(&sand[i]);
  }

  // Update current sand stats
  ParticleStats p;
  p.pos_x = sand[0].position.x;
  p.pos_y = sand[0].position.y;
  p.vel_x = sand[0].acceleration.x;
  p.vel_y = sand[0].acceleration.y;
  settings.particle.AddParticle(&p);
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
      if ((x % (FB_CONTAINER_OUTPUT / settings.fluid_holes) == 0 &&
           y == FB_CONTAINER_OUTPUT - 1)) {
        colours[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3)] = blue.r;
        colours[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3) + 1] = blue.b;
        colours[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3) + 2] = blue.g;
      } else {
        colours[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3)] = grey.r;
        colours[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3) + 1] = grey.b;
        colours[(y * FB_CONTAINER_OUTPUT * 3) + (x * 3) + 2] = grey.g;
      }
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
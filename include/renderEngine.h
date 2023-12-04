#pragma once

#include <string>
#include <vector>

#include "VectorMath.h"

struct Colour3 {
  float r = 0;
  float b = 0;
  float g = 0;
  Colour3(float red, float green, float blue) {
    r = red;
    b = blue;
    g = green;
  }
};

struct ParticleStats {
  double pos_x;
  double pos_y;

  double vel_x;
  double vel_y;
};

class FluidEngineParticleStats {
 private:
  std::vector<ParticleStats> m_stats;
  int m_max = 10;

 public:
  std::vector<ParticleStats> GetStats() const { return m_stats; }
  int GetMax() const { return m_max; }
  inline void AddParticle(ParticleStats* stat) {
    m_stats.push_back(*stat);
    if (m_stats.size() > m_max) {
      m_stats.erase(m_stats.begin());
    }
  };
};

struct FluidEngineSettings {
  float gravity = 0.5;
  float dampen = 0.5;
  float heat = 0;
  int fluid_holes = 20;
  float fluid_power = 1;
  double mass = 0.02;
  double radius = 0.0003;
  double dragCoefficient = 0.47;
  int collisionCalcCount = 10;
  bool useNormalGravity = true;
  double fluidDensity = 1.204;
};

class renderEngine {
 public:
  renderEngine();
  ~renderEngine();

  void Initialise(const char* title, int w, int h);
  void LinkSettings(FluidEngineSettings* set) { settings = set; };

  void Update();
  void Render();
  void Clean();
  bool Running() { return isRunning; };
  std::vector<std::string> currentDebugInfo;

 private:
  FluidEngineSettings* settings;
  int tick = 0;
  bool isRunning;
};
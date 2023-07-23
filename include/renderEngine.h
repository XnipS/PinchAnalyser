#pragma once

#include <string>
#include <vector>

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

struct FluidEngineSettings {
  float gravity = 0.5;
  float dampen = 0.5;
  float heat = 0;
  int fluid_holes = 20;
  float fluid_power = 1;
  float bounce_threshold = 0.1;
  double mass = 0.02;
  double radius = 0.0003;
  double dragCoefficient = 0.47;
};

class renderEngine {
 public:
  renderEngine();
  ~renderEngine();

  void Initialise(const char* title, int w, int h);
  void LinkSettings(FluidEngineSettings* set) { settings = set; };
  void UpdateImage(float* colours);
  void FloodImage(Colour3 col);
  void Update();
  void Render();
  void Clean();
  bool Running() { return isRunning; };
  std::vector<std::string> currentDebugInfo;
  int val_totalSand;
  int AddSand() { return addSand; };
  bool ClearSand() { return clearAllSand; };

 private:
  FluidEngineSettings* settings;
  int tick = 0;
  bool isRunning;
  int addSand = 0;
  bool clearAllSand = false;
};
#pragma once
#include "../depend/imgui/backends/imgui_impl_opengl3.h"
#include "../depend/imgui/backends/imgui_impl_sdl2.h"
#include "../depend/imgui/imgui.h"

struct Colour3 {
  float r = 0;
  float b = 0;
  float g = 0;
};

class renderEngine {
 public:
  renderEngine();
  ~renderEngine();

  void Initialise(const char *title, int w, int h);
  void UpdateImage(float colours[]);
  void FloodImage(Colour3 col);
  void Update();
  void Render();
  void Clean();
  bool Running() { return isRunning; };

 private:
  int tick = 0;
  bool isRunning;
};
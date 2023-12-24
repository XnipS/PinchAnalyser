#pragma once

#include <vector>

#include "exchangerEngine.h"

class renderEngine {
 public:
  renderEngine();
  ~renderEngine();

  void Initialise(const char* title, int w, int h);

  void Update();
  void Render();
  void Clean();
  bool Running() { return isRunning; };

 private:
  std::vector<inputStream> inputStreams;
  int tick = 0;
  bool isRunning;
};